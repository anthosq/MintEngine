# RenderGraph 完全指南

> 从传统渲染管线的痛点出发，深入理解 RenderGraph 的设计动机、核心概念与实现方法

---

## 目录

1. [为什么需要 RenderGraph](#1-为什么需要-rendergraph)
2. [RenderGraph 核心概念](#2-rendergraph-核心概念)
3. [设计原理深度解析](#3-设计原理深度解析)
4. [完整实现方案](#4-完整实现方案)
5. [使用示例与最佳实践](#5-使用示例与最佳实践)
6. [开发规划与里程碑](#6-开发规划与里程碑)
7. [进阶主题](#7-进阶主题)
8. [参考资源](#8-参考资源)

---

# 1. 为什么需要 RenderGraph

## 1.1 传统渲染管线的问题

在理解 RenderGraph 之前，我们先看看传统渲染管线面临的挑战：

### 问题一：手动资源管理的复杂性

```cpp
// 传统方式：手动管理每个 RenderTarget
class TraditionalRenderer {
    Ref<Framebuffer> m_gbuffer;
    Ref<Framebuffer> m_shadowMap;
    Ref<Framebuffer> m_ssaoBuffer;
    Ref<Framebuffer> m_bloomBuffer;
    Ref<Framebuffer> m_finalBuffer;
    // ... 更多 buffer

    void Render() {
        // 必须手动确保正确的执行顺序
        RenderShadowMap();      // 1. 先渲染阴影
        RenderGBuffer();        // 2. 再渲染 GBuffer
        RenderSSAO();           // 3. SSAO 依赖 GBuffer 深度
        RenderLighting();       // 4. 光照依赖 GBuffer + Shadow + SSAO
        RenderBloom();          // 5. Bloom 依赖光照结果
        RenderFinal();          // 6. 最终合成
    }
};
```

**痛点**：
- 添加新 Pass 需要手动调整执行顺序
- 资源依赖关系隐式存在于代码顺序中，难以维护
- 资源生命周期管理混乱，容易内存泄漏或过早释放

### 问题二：资源浪费

```cpp
// 传统方式：所有 Buffer 在整个帧周期内都存在
void TraditionalRenderer::Init() {
    // 所有资源在引擎启动时创建，运行期间一直占用显存
    m_gbuffer = Framebuffer::Create({1920, 1080, ...});     // ~50MB
    m_shadowMap = Framebuffer::Create({4096, 4096, ...});   // ~64MB
    m_ssaoBuffer = Framebuffer::Create({1920, 1080, ...});  // ~8MB
    m_bloomBuffer = Framebuffer::Create({...});             // ~24MB
    // 总计: 146MB+ 显存，即使某些 Pass 可以复用内存
}
```

**痛点**：
- GBuffer 只在 GBuffer Pass 和 Lighting Pass 之间需要
- Shadow Map 渲染完成后，只需要在 Lighting 时读取
- 很多资源的生命周期实际上不重叠，理论上可以共享内存

### 问题三：同步与并行的困难

```cpp
// 传统方式：完全串行执行
void TraditionalRenderer::Render() {
    // GPU 在等待 CPU 提交命令
    // CPU 在等待 GPU 完成上一步

    BeginPass(shadowPass);
    DrawShadowCasters();
    EndPass();  // 隐式等待完成

    BeginPass(gbufferPass);
    DrawScene();
    EndPass();  // 隐式等待完成

    // ... 每一步都在等待
}
```

**痛点**：
- 现代 GPU 支持 Graphics 和 Compute 并行执行
- 传统方式难以利用异步计算能力
- 手动管理同步点极易出错

### 问题四：代码耦合严重

```cpp
// 传统方式：Pass 之间直接依赖
class LightingPass {
    void Execute() {
        // 直接访问其他 Pass 的资源
        auto gbuffer = m_gbufferPass->GetOutput();    // 紧耦合
        auto shadowMap = m_shadowPass->GetOutput();   // 紧耦合
        auto ssao = m_ssaoPass->GetOutput();          // 紧耦合
        // ...
    }
};
```

**痛点**：
- Pass 之间相互引用，难以单独测试
- 添加/移除 Pass 需要修改多处代码
- 难以实现 Pass 的动态组合

---

## 1.2 RenderGraph 的解决方案

RenderGraph 通过**声明式编程**和**延迟执行**来解决这些问题：

```cpp
// RenderGraph 方式：声明式定义
void SetupFrame(RenderGraph& graph) {
    // 只声明"需要什么"，不关心"怎么做"
    auto shadowMap = graph.CreateTexture("ShadowMap", shadowDesc);
    auto gbuffer = graph.CreateTexture("GBuffer", gbufferDesc);
    auto lighting = graph.CreateTexture("Lighting", lightingDesc);

    graph.AddPass("Shadow", [&](PassBuilder& builder) {
        builder.Write(shadowMap);
    }, [](Context& ctx) { /* 渲染阴影 */ });

    graph.AddPass("GBuffer", [&](PassBuilder& builder) {
        builder.Write(gbuffer);
    }, [](Context& ctx) { /* 渲染几何 */ });

    graph.AddPass("Lighting", [&](PassBuilder& builder) {
        builder.Read(shadowMap);  // 显式声明依赖
        builder.Read(gbuffer);    // 显式声明依赖
        builder.Write(lighting);
    }, [](Context& ctx) { /* 计算光照 */ });

    // 框架自动：
    // 1. 分析依赖关系
    // 2. 确定执行顺序
    // 3. 管理资源生命周期
    // 4. 优化内存使用
    graph.Compile();
    graph.Execute();
}
```

---

## 1.3 RenderGraph 的核心优势

| 传统方式 | RenderGraph |
|----------|-------------|
| 手动管理执行顺序 | 自动拓扑排序 |
| 资源全帧存在 | 按需创建/销毁 |
| 隐式依赖关系 | 显式依赖声明 |
| 难以并行化 | 自动并行分析 |
| 紧耦合 | 松耦合、可组合 |
| 调试困难 | 可视化依赖图 |

---

# 2. RenderGraph 核心概念

## 2.1 概念总览

```
┌─────────────────────────────────────────────────────────────────────────┐
│                           RenderGraph                                    │
│                                                                          │
│  ┌─────────────┐    ┌─────────────┐    ┌─────────────┐                 │
│  │  Resource   │    │    Pass     │    │  Execution  │                 │
│  │  (资源)     │    │   (Pass)    │    │   (执行)    │                 │
│  ├─────────────┤    ├─────────────┤    ├─────────────┤                 │
│  │ • Transient │    │ • Setup     │    │ • Compile   │                 │
│  │ • Imported  │    │ • Execute   │    │ • Execute   │                 │
│  │ • Handle    │    │ • Read/Write│    │ • Validate  │                 │
│  └─────────────┘    └─────────────┘    └─────────────┘                 │
│                                                                          │
└─────────────────────────────────────────────────────────────────────────┘
```

## 2.2 资源 (Resource)

### 资源句柄 (Resource Handle)

资源句柄是一个轻量级标识符，代表一个**虚拟资源**：

```cpp
// 资源句柄：不直接持有 GPU 资源，只是一个 ID
struct RGHandle {
    uint32_t index;      // 资源在数组中的索引
    uint32_t version;    // 版本号，用于检测失效

    bool IsValid() const { return index != INVALID_INDEX; }

    bool operator==(const RGHandle& other) const {
        return index == other.index && version == other.version;
    }
};

// 类型安全的句柄
struct RGTextureHandle : RGHandle {};
struct RGBufferHandle : RGHandle {};
```

**为什么使用句柄而不是指针？**
- 句柄可以表示"尚未创建"的资源
- 句柄可以被验证（通过 version）
- 句柄便于序列化和调试

### 资源类型

```cpp
// 瞬态资源 (Transient): 由 RenderGraph 创建和管理
auto hdrBuffer = graph.CreateTexture("HDR", {
    .width = 1920,
    .height = 1080,
    .format = TextureFormat::RGBA16F
});

// 导入资源 (Imported): 外部资源，生命周期由外部管理
auto backbuffer = graph.ImportTexture("Backbuffer", swapchainTexture);
auto sceneData = graph.ImportBuffer("SceneUBO", uniformBuffer);
```

### 资源描述

```cpp
struct RGTextureDesc {
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t depth = 1;
    uint32_t mipLevels = 1;
    uint32_t arrayLayers = 1;

    TextureFormat format = TextureFormat::RGBA8;
    TextureType type = TextureType::Texture2D;
    TextureUsage usage = TextureUsage::RenderTarget;

    // 用于调试
    std::string debugName;

    // 计算资源大小（用于 aliasing 决策）
    size_t CalculateSize() const;
};

struct RGBufferDesc {
    size_t size = 0;
    BufferUsage usage = BufferUsage::Uniform;
    std::string debugName;
};
```

## 2.3 Pass

Pass 是渲染图中的执行单元，由两部分组成：

### Setup 阶段：声明资源依赖

```cpp
graph.AddPass("MyPass",
    // Setup Lambda: 在 Compile 之前执行，声明资源访问
    [&](RGPassBuilder& builder) {
        // 读取资源：建立依赖关系
        builder.Read(inputTexture, RGReadAccess::Shader);

        // 写入资源：声明输出
        builder.Write(outputTexture, RGWriteAccess::RenderTarget);

        // 创建瞬态资源
        auto temp = builder.CreateTexture("Temp", tempDesc);
        builder.Write(temp);

        // 设置 Pass 属性
        builder.SetQueue(RGQueue::Graphics);  // 或 Compute
    },
    // Execute Lambda: 在运行时执行，实际渲染逻辑
    [=](RGContext& ctx) {
        // 获取实际的 GPU 资源
        auto* input = ctx.GetTexture(inputTexture);
        auto* output = ctx.GetTexture(outputTexture);

        // 执行渲染命令
        ctx.SetRenderTarget(output);
        ctx.BindTexture(0, input);
        ctx.DrawFullscreenQuad();
    }
);
```

### 资源访问类型

```cpp
enum class RGReadAccess {
    Shader,         // 在着色器中采样
    CopySource,     // 作为复制源
    Present         // 用于呈现
};

enum class RGWriteAccess {
    RenderTarget,   // 作为渲染目标
    DepthStencil,   // 作为深度/模板
    CopyDest,       // 作为复制目标
    UAV             // 作为 UAV (Compute)
};
```

## 2.4 依赖关系

### 显式依赖 vs 隐式依赖

```cpp
// 显式依赖：通过 Read/Write 声明
graph.AddPass("A", [&](auto& b) { b.Write(tex); }, ...);
graph.AddPass("B", [&](auto& b) { b.Read(tex); }, ...);
// B 显式依赖 A（因为 B 读取 A 写入的资源）

// 隐式依赖：框架自动推导
// 如果 Pass C 读取 tex，但 tex 被 A 和 B 都写入过
// 框架会自动确定 C 依赖"最后一个"写入 tex 的 Pass
```

### 依赖图示例

```
         ┌──────────────┐
         │ ShadowPass   │
         │ Write: Shadow│
         └──────┬───────┘
                │
         ┌──────▼───────┐
         │ GBufferPass  │
         │ Write: GBuffer│
         └──────┬───────┘
                │
    ┌───────────┴───────────┐
    │                       │
┌───▼────────┐      ┌───────▼──────┐
│ SSAOPass   │      │ Shadow lookup│
│ Read: Depth│      │ Read: Shadow │
│Write: SSAO │      └───────┬──────┘
└───┬────────┘              │
    │                       │
    └───────────┬───────────┘
                │
         ┌──────▼───────┐
         │ LightingPass │
         │ Read: GBuffer│
         │ Read: Shadow │
         │ Read: SSAO   │
         │ Write: HDR   │
         └──────┬───────┘
                │
         ┌──────▼───────┐
         │ PostProcess  │
         │ Read: HDR    │
         │Write: Final  │
         └──────────────┘
```

---

# 3. 设计原理深度解析

## 3.1 编译阶段 (Compile)

编译阶段是 RenderGraph 的核心，负责：
1. 构建依赖图
2. 拓扑排序确定执行顺序
3. 计算资源生命周期
4. 执行资源别名优化

### 3.1.1 依赖图构建

```cpp
void RenderGraph::BuildDependencyGraph() {
    // 遍历所有 Pass
    for (auto& pass : m_passes) {
        // 对于每个读取的资源
        for (auto& read : pass.reads) {
            auto& resource = m_resources[read.handle.index];

            // 如果这个资源之前被某个 Pass 写入过
            if (resource.lastWriter != INVALID_PASS) {
                // 建立依赖边：当前 Pass 依赖于 lastWriter
                m_dependencies[pass.index].push_back(resource.lastWriter);
            }
        }

        // 对于每个写入的资源，更新 lastWriter
        for (auto& write : pass.writes) {
            m_resources[write.handle.index].lastWriter = pass.index;
        }
    }
}
```

### 3.1.2 拓扑排序

```cpp
std::vector<uint32_t> RenderGraph::TopologicalSort() {
    std::vector<uint32_t> result;
    std::vector<int> inDegree(m_passes.size(), 0);
    std::queue<uint32_t> readyQueue;

    // 计算入度（有多少 Pass 依赖于当前 Pass）
    for (uint32_t i = 0; i < m_passes.size(); i++) {
        inDegree[i] = m_dependencies[i].size();
        if (inDegree[i] == 0) {
            readyQueue.push(i);
        }
    }

    // Kahn 算法
    while (!readyQueue.empty()) {
        uint32_t current = readyQueue.front();
        readyQueue.pop();
        result.push_back(current);

        // 减少依赖当前 Pass 的其他 Pass 的入度
        for (uint32_t dependent : m_reverseDependencies[current]) {
            if (--inDegree[dependent] == 0) {
                readyQueue.push(dependent);
            }
        }
    }

    // 检测循环依赖
    if (result.size() != m_passes.size()) {
        MINT_ERROR("RenderGraph has cyclic dependency!");
        return {};
    }

    return result;
}
```

### 3.1.3 资源生命周期分析

```cpp
void RenderGraph::CalculateResourceLifetimes() {
    // 对于每个资源，找到首次使用和最后使用的 Pass
    for (auto& resource : m_resources) {
        resource.firstUse = UINT32_MAX;
        resource.lastUse = 0;
    }

    // 按执行顺序遍历
    for (uint32_t order = 0; order < m_executionOrder.size(); order++) {
        uint32_t passIdx = m_executionOrder[order];
        auto& pass = m_passes[passIdx];

        // 更新读取资源的生命周期
        for (auto& read : pass.reads) {
            auto& res = m_resources[read.handle.index];
            res.firstUse = std::min(res.firstUse, order);
            res.lastUse = std::max(res.lastUse, order);
        }

        // 更新写入资源的生命周期
        for (auto& write : pass.writes) {
            auto& res = m_resources[write.handle.index];
            res.firstUse = std::min(res.firstUse, order);
            res.lastUse = std::max(res.lastUse, order);
        }
    }
}
```

## 3.2 资源别名 (Aliasing)

资源别名是 RenderGraph 最强大的优化之一：**生命周期不重叠的资源可以共享同一块内存**。

### 3.2.1 别名原理

```
时间轴:     Pass0   Pass1   Pass2   Pass3   Pass4   Pass5
          ├───────┼───────┼───────┼───────┼───────┼───────┤
资源A:     [███████████████]                              (Pass0-Pass2)
资源B:                             [███████████████]      (Pass3-Pass5)
资源C:             [███████]                              (Pass1-Pass2)

优化后内存布局:
          ├───────────────────────────────────────────────┤
内存块1:   [  资源A  ] ──────────→ [  资源B  ]            (A和B可共享)
内存块2:   [    ][资源C][    ]                            (C单独)
```

### 3.2.2 别名算法

```cpp
void RenderGraph::ComputeResourceAliasing() {
    // 按资源大小排序（大资源优先分配）
    std::vector<uint32_t> sortedResources = SortResourcesBySize();

    // 内存池：记录每块内存的使用时间段
    struct MemoryBlock {
        size_t size;
        std::vector<std::pair<uint32_t, uint32_t>> usedRanges; // (start, end)
    };
    std::vector<MemoryBlock> memoryPool;

    for (uint32_t resIdx : sortedResources) {
        auto& resource = m_resources[resIdx];

        // 跳过导入资源（不能被别名）
        if (resource.isImported) continue;

        // 尝试找到可复用的内存块
        bool found = false;
        for (auto& block : memoryPool) {
            if (block.size >= resource.GetSize() &&
                !OverlapsWithAny(block.usedRanges, resource.firstUse, resource.lastUse)) {
                // 可以复用这块内存
                resource.aliasedMemory = &block;
                block.usedRanges.push_back({resource.firstUse, resource.lastUse});
                found = true;
                break;
            }
        }

        // 没找到可复用的，创建新内存块
        if (!found) {
            memoryPool.push_back({resource.GetSize(), {{resource.firstUse, resource.lastUse}}});
            resource.aliasedMemory = &memoryPool.back();
        }
    }
}
```

### 3.2.3 别名效果示例

```
未优化:
┌────────────────────────────────────────────────────┐
│ GBuffer Albedo    : 1920x1080 RGBA8    = 8.3 MB    │
│ GBuffer Normal    : 1920x1080 RGB16F   = 12.4 MB   │
│ GBuffer Depth     : 1920x1080 D24S8    = 8.3 MB    │
│ Shadow Map        : 4096x4096 D32      = 67.1 MB   │
│ SSAO Buffer       : 960x540 R8         = 0.5 MB    │
│ HDR Buffer        : 1920x1080 RGBA16F  = 16.6 MB   │
│ Bloom Temp        : 960x540 RGBA16F    = 4.1 MB    │
│ Final Output      : 1920x1080 RGBA8    = 8.3 MB    │
├────────────────────────────────────────────────────┤
│ 总计: 125.6 MB                                     │
└────────────────────────────────────────────────────┘

启用别名后:
┌────────────────────────────────────────────────────┐
│ 内存块 1 (67.1 MB): Shadow Map → (复用) HDR Buffer │
│ 内存块 2 (12.4 MB): GBuffer Normal → Bloom Temp   │
│ 内存块 3 (8.3 MB) : GBuffer Albedo → Final Output │
│ 内存块 4 (8.3 MB) : GBuffer Depth (全程使用)      │
│ 内存块 5 (0.5 MB) : SSAO Buffer                   │
├────────────────────────────────────────────────────┤
│ 总计: 96.6 MB (节省 23%)                          │
└────────────────────────────────────────────────────┘
```

## 3.3 同步与屏障 (Barriers)

在 Vulkan/DX12 等现代 API 中，需要显式管理资源状态转换：

### 3.3.1 资源状态

```cpp
enum class ResourceState {
    Undefined,          // 初始状态
    RenderTarget,       // 作为渲染目标
    DepthWrite,         // 深度写入
    DepthRead,          // 深度只读
    ShaderResource,     // 着色器采样
    UnorderedAccess,    // UAV 访问
    CopySource,         // 复制源
    CopyDest,           // 复制目标
    Present             // 呈现
};
```

### 3.3.2 自动屏障插入

```cpp
void RenderGraph::InsertBarriers() {
    for (uint32_t order = 0; order < m_executionOrder.size(); order++) {
        auto& pass = m_passes[m_executionOrder[order]];

        // 为每个读取的资源检查状态转换
        for (auto& read : pass.reads) {
            auto& resource = m_resources[read.handle.index];
            ResourceState requiredState = GetStateForReadAccess(read.access);

            if (resource.currentState != requiredState) {
                // 需要插入屏障
                pass.barriers.push_back({
                    .resource = read.handle,
                    .before = resource.currentState,
                    .after = requiredState
                });
                resource.currentState = requiredState;
            }
        }

        // 为每个写入的资源检查状态转换
        for (auto& write : pass.writes) {
            auto& resource = m_resources[write.handle.index];
            ResourceState requiredState = GetStateForWriteAccess(write.access);

            if (resource.currentState != requiredState) {
                pass.barriers.push_back({
                    .resource = write.handle,
                    .before = resource.currentState,
                    .after = requiredState
                });
                resource.currentState = requiredState;
            }
        }
    }
}
```

---

# 4. 完整实现方案

## 4.1 核心类定义

```cpp
// ================== rg_types.h ==================

#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <functional>

namespace Mint {

// 无效索引标记
constexpr uint32_t RG_INVALID_INDEX = UINT32_MAX;

// ============ 资源句柄 ============
struct RGHandle {
    uint32_t index = RG_INVALID_INDEX;
    uint32_t version = 0;

    bool IsValid() const { return index != RG_INVALID_INDEX; }
    operator bool() const { return IsValid(); }
};

struct RGTextureHandle : RGHandle {};
struct RGBufferHandle : RGHandle {};

// ============ 资源描述 ============
enum class RGTextureFormat {
    Unknown,
    R8, RG8, RGBA8, RGBA8_SRGB,
    R16F, RG16F, RGBA16F,
    R32F, RG32F, RGBA32F,
    Depth16, Depth24, Depth32F,
    Depth24Stencil8
};

enum class RGTextureUsage : uint32_t {
    None = 0,
    RenderTarget = 1 << 0,
    DepthStencil = 1 << 1,
    ShaderResource = 1 << 2,
    UnorderedAccess = 1 << 3
};

struct RGTextureDesc {
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t depth = 1;
    uint32_t mipLevels = 1;

    RGTextureFormat format = RGTextureFormat::RGBA8;
    RGTextureUsage usage = RGTextureUsage::ShaderResource;

    std::string name;

    size_t GetSizeInBytes() const;
};

struct RGBufferDesc {
    size_t size = 0;
    std::string name;
};

// ============ 访问类型 ============
enum class RGReadAccess {
    Shader,
    CopySource
};

enum class RGWriteAccess {
    RenderTarget,
    DepthStencil,
    CopyDest,
    UnorderedAccess
};

// ============ Pass 队列类型 ============
enum class RGQueue {
    Graphics,
    Compute,
    Copy
};

} // namespace Mint
```

## 4.2 资源管理

```cpp
// ================== rg_resource.h ==================

#pragma once
#include "rg_types.h"
#include "render/texture.h"
#include "render/buffer.h"

namespace Mint {

// 资源读取记录
struct RGResourceRead {
    RGHandle handle;
    RGReadAccess access;
};

// 资源写入记录
struct RGResourceWrite {
    RGHandle handle;
    RGWriteAccess access;
};

// 虚拟纹理资源
struct RGTexture {
    RGTextureDesc desc;

    // 生命周期
    uint32_t firstUsePass = RG_INVALID_INDEX;
    uint32_t lastUsePass = RG_INVALID_INDEX;

    // 写入追踪
    uint32_t lastWriter = RG_INVALID_INDEX;

    // 物理资源（延迟创建）
    Ref<Texture> physicalTexture;

    // 是否是导入资源
    bool isImported = false;

    // 引用计数（在当前帧中）
    uint32_t refCount = 0;
};

// 虚拟缓冲区资源
struct RGBuffer {
    RGBufferDesc desc;

    uint32_t firstUsePass = RG_INVALID_INDEX;
    uint32_t lastUsePass = RG_INVALID_INDEX;
    uint32_t lastWriter = RG_INVALID_INDEX;

    Ref<Buffer> physicalBuffer;
    bool isImported = false;
    uint32_t refCount = 0;
};

} // namespace Mint
```

## 4.3 Pass 定义

```cpp
// ================== rg_pass.h ==================

#pragma once
#include "rg_types.h"
#include "rg_resource.h"

namespace Mint {

class RenderGraph;
class RGContext;

// Pass 构建器：用于在 Setup 阶段声明资源依赖
class RGPassBuilder {
public:
    RGPassBuilder(RenderGraph& graph, uint32_t passIndex);

    // 读取资源
    void Read(RGTextureHandle handle, RGReadAccess access = RGReadAccess::Shader);
    void Read(RGBufferHandle handle, RGReadAccess access = RGReadAccess::Shader);

    // 写入资源
    void Write(RGTextureHandle handle, RGWriteAccess access = RGWriteAccess::RenderTarget);
    void Write(RGBufferHandle handle, RGWriteAccess access = RGWriteAccess::UnorderedAccess);

    // 创建瞬态资源（仅在当前 Pass 使用）
    RGTextureHandle CreateTexture(const std::string& name, const RGTextureDesc& desc);
    RGBufferHandle CreateBuffer(const std::string& name, const RGBufferDesc& desc);

    // 设置 Pass 属性
    void SetQueue(RGQueue queue);
    void SetName(const std::string& name);

private:
    RenderGraph& m_graph;
    uint32_t m_passIndex;
};

// Pass 执行上下文：用于在 Execute 阶段访问实际资源
class RGContext {
public:
    RGContext(RenderGraph& graph);

    // 获取实际的 GPU 资源
    Texture* GetTexture(RGTextureHandle handle);
    Buffer* GetBuffer(RGBufferHandle handle);

    // 渲染命令（简化版，实际应该更完整）
    void SetRenderTarget(RGTextureHandle color, RGTextureHandle depth = {});
    void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
    void BindTexture(uint32_t slot, RGTextureHandle handle);
    void BindBuffer(uint32_t slot, RGBufferHandle handle);
    void DrawFullscreenQuad();
    void Draw(uint32_t vertexCount, uint32_t firstVertex = 0);
    void DrawIndexed(uint32_t indexCount, uint32_t firstIndex = 0);

private:
    RenderGraph& m_graph;
};

// Pass 定义
using RGSetupFunc = std::function<void(RGPassBuilder&)>;
using RGExecuteFunc = std::function<void(RGContext&)>;

struct RGPass {
    std::string name;
    RGQueue queue = RGQueue::Graphics;

    // 资源依赖
    std::vector<RGResourceRead> reads;
    std::vector<RGResourceWrite> writes;

    // 执行回调
    RGExecuteFunc executeFunc;

    // 依赖关系（编译后填充）
    std::vector<uint32_t> dependencies;

    // 是否被裁剪（输出未被使用）
    bool culled = false;
};

} // namespace Mint
```

## 4.4 RenderGraph 主类

```cpp
// ================== render_graph.h ==================

#pragma once
#include "rg_pass.h"
#include <unordered_map>

namespace Mint {

class RenderGraph {
public:
    RenderGraph() = default;
    ~RenderGraph() = default;

    // 禁止拷贝
    RenderGraph(const RenderGraph&) = delete;
    RenderGraph& operator=(const RenderGraph&) = delete;

    // ============ 资源创建 ============

    // 创建瞬态纹理（由 RenderGraph 管理生命周期）
    RGTextureHandle CreateTexture(const std::string& name, const RGTextureDesc& desc);

    // 创建瞬态缓冲区
    RGBufferHandle CreateBuffer(const std::string& name, const RGBufferDesc& desc);

    // 导入外部纹理（生命周期由外部管理）
    RGTextureHandle ImportTexture(const std::string& name, Ref<Texture> texture);

    // 导入外部缓冲区
    RGBufferHandle ImportBuffer(const std::string& name, Ref<Buffer> buffer);

    // ============ Pass 注册 ============

    template<typename SetupFunc, typename ExecuteFunc>
    void AddPass(const std::string& name, SetupFunc&& setup, ExecuteFunc&& execute) {
        uint32_t passIndex = static_cast<uint32_t>(m_passes.size());
        m_passes.emplace_back();

        RGPass& pass = m_passes.back();
        pass.name = name;
        pass.executeFunc = std::forward<ExecuteFunc>(execute);

        // 执行 Setup
        RGPassBuilder builder(*this, passIndex);
        setup(builder);
    }

    // ============ 编译与执行 ============

    // 编译：分析依赖、计算执行顺序、资源生命周期
    void Compile();

    // 执行所有 Pass
    void Execute();

    // 重置（每帧开始时调用）
    void Reset();

    // ============ 调试 ============

    // 导出依赖图（DOT 格式，可用 Graphviz 可视化）
    std::string ExportGraphviz() const;

    // 打印资源使用统计
    void PrintStatistics() const;

private:
    friend class RGPassBuilder;
    friend class RGContext;

    // 内部方法
    void BuildDependencyGraph();
    void TopologicalSort();
    void CalculateResourceLifetimes();
    void CullUnusedPasses();
    void AllocateResources();
    void ReleaseResources();

    // 资源存储
    std::vector<RGTexture> m_textures;
    std::vector<RGBuffer> m_buffers;

    // Pass 存储
    std::vector<RGPass> m_passes;

    // 执行顺序（编译后填充）
    std::vector<uint32_t> m_executionOrder;

    // 名称映射（用于调试）
    std::unordered_map<std::string, RGTextureHandle> m_textureNameMap;
    std::unordered_map<std::string, RGBufferHandle> m_bufferNameMap;

    // 状态
    bool m_compiled = false;

    // 统计
    struct Statistics {
        size_t totalTextureMemory = 0;
        size_t aliasedMemorySaved = 0;
        uint32_t passCount = 0;
        uint32_t culledPassCount = 0;
    } m_stats;
};

} // namespace Mint
```

## 4.5 实现细节

```cpp
// ================== render_graph.cpp ==================

#include "render_graph.h"
#include <queue>
#include <algorithm>
#include <sstream>

namespace Mint {

// ============ 资源创建 ============

RGTextureHandle RenderGraph::CreateTexture(const std::string& name, const RGTextureDesc& desc) {
    RGTextureHandle handle;
    handle.index = static_cast<uint32_t>(m_textures.size());
    handle.version = 0;

    RGTexture texture;
    texture.desc = desc;
    texture.desc.name = name;
    texture.isImported = false;
    m_textures.push_back(texture);

    m_textureNameMap[name] = handle;
    return handle;
}

RGTextureHandle RenderGraph::ImportTexture(const std::string& name, Ref<Texture> texture) {
    RGTextureHandle handle;
    handle.index = static_cast<uint32_t>(m_textures.size());
    handle.version = 0;

    RGTexture rgTexture;
    rgTexture.desc.name = name;
    rgTexture.desc.width = texture->GetWidth();
    rgTexture.desc.height = texture->GetHeight();
    rgTexture.physicalTexture = texture;
    rgTexture.isImported = true;
    m_textures.push_back(rgTexture);

    m_textureNameMap[name] = handle;
    return handle;
}

// ============ Pass Builder ============

RGPassBuilder::RGPassBuilder(RenderGraph& graph, uint32_t passIndex)
    : m_graph(graph), m_passIndex(passIndex) {}

void RGPassBuilder::Read(RGTextureHandle handle, RGReadAccess access) {
    auto& pass = m_graph.m_passes[m_passIndex];
    pass.reads.push_back({handle, access});

    // 增加资源引用计数
    m_graph.m_textures[handle.index].refCount++;
}

void RGPassBuilder::Write(RGTextureHandle handle, RGWriteAccess access) {
    auto& pass = m_graph.m_passes[m_passIndex];
    pass.writes.push_back({handle, access});

    // 更新资源的最后写入者
    m_graph.m_textures[handle.index].lastWriter = m_passIndex;
    m_graph.m_textures[handle.index].refCount++;
}

// ============ 编译 ============

void RenderGraph::Compile() {
    if (m_compiled) return;

    // 1. 构建依赖图
    BuildDependencyGraph();

    // 2. 拓扑排序
    TopologicalSort();

    // 3. 计算资源生命周期
    CalculateResourceLifetimes();

    // 4. 裁剪未使用的 Pass（可选）
    CullUnusedPasses();

    // 5. 分配物理资源
    AllocateResources();

    m_compiled = true;
}

void RenderGraph::BuildDependencyGraph() {
    for (uint32_t passIdx = 0; passIdx < m_passes.size(); passIdx++) {
        auto& pass = m_passes[passIdx];

        // 对于每个读取的纹理资源
        for (auto& read : pass.reads) {
            auto& texture = m_textures[read.handle.index];

            // 如果资源之前被写入过，建立依赖
            if (texture.lastWriter != RG_INVALID_INDEX &&
                texture.lastWriter != passIdx) {
                pass.dependencies.push_back(texture.lastWriter);
            }
        }

        // 去重依赖
        std::sort(pass.dependencies.begin(), pass.dependencies.end());
        pass.dependencies.erase(
            std::unique(pass.dependencies.begin(), pass.dependencies.end()),
            pass.dependencies.end()
        );
    }
}

void RenderGraph::TopologicalSort() {
    std::vector<uint32_t> inDegree(m_passes.size(), 0);
    std::vector<std::vector<uint32_t>> dependents(m_passes.size());

    // 计算入度和反向依赖
    for (uint32_t i = 0; i < m_passes.size(); i++) {
        inDegree[i] = static_cast<uint32_t>(m_passes[i].dependencies.size());
        for (uint32_t dep : m_passes[i].dependencies) {
            dependents[dep].push_back(i);
        }
    }

    // Kahn 算法
    std::queue<uint32_t> readyQueue;
    for (uint32_t i = 0; i < m_passes.size(); i++) {
        if (inDegree[i] == 0) {
            readyQueue.push(i);
        }
    }

    m_executionOrder.clear();
    while (!readyQueue.empty()) {
        uint32_t current = readyQueue.front();
        readyQueue.pop();
        m_executionOrder.push_back(current);

        for (uint32_t dependent : dependents[current]) {
            if (--inDegree[dependent] == 0) {
                readyQueue.push(dependent);
            }
        }
    }

    // 检测循环依赖
    if (m_executionOrder.size() != m_passes.size()) {
        MINT_CORE_ERROR("RenderGraph: Cyclic dependency detected!");
    }
}

void RenderGraph::CalculateResourceLifetimes() {
    // 按执行顺序遍历，更新资源的首次/最后使用
    for (uint32_t order = 0; order < m_executionOrder.size(); order++) {
        uint32_t passIdx = m_executionOrder[order];
        auto& pass = m_passes[passIdx];

        for (auto& read : pass.reads) {
            auto& tex = m_textures[read.handle.index];
            if (tex.firstUsePass == RG_INVALID_INDEX) {
                tex.firstUsePass = order;
            }
            tex.lastUsePass = order;
        }

        for (auto& write : pass.writes) {
            auto& tex = m_textures[write.handle.index];
            if (tex.firstUsePass == RG_INVALID_INDEX) {
                tex.firstUsePass = order;
            }
            tex.lastUsePass = order;
        }
    }
}

void RenderGraph::AllocateResources() {
    for (auto& texture : m_textures) {
        if (!texture.isImported && !texture.physicalTexture) {
            // 创建物理纹理
            TextureSpecification spec;
            spec.Width = texture.desc.width;
            spec.Height = texture.desc.height;
            // ... 转换格式等
            texture.physicalTexture = Texture2D::Create(spec);

            m_stats.totalTextureMemory += texture.desc.GetSizeInBytes();
        }
    }
}

// ============ 执行 ============

void RenderGraph::Execute() {
    if (!m_compiled) {
        Compile();
    }

    RGContext context(*this);

    for (uint32_t passIdx : m_executionOrder) {
        auto& pass = m_passes[passIdx];

        if (pass.culled) continue;

        // 执行 Pass
        if (pass.executeFunc) {
            pass.executeFunc(context);
        }
    }
}

// ============ 调试 ============

std::string RenderGraph::ExportGraphviz() const {
    std::stringstream ss;
    ss << "digraph RenderGraph {\n";
    ss << "    rankdir=TB;\n";
    ss << "    node [shape=box];\n\n";

    // 输出 Pass 节点
    for (uint32_t i = 0; i < m_passes.size(); i++) {
        auto& pass = m_passes[i];
        ss << "    pass_" << i << " [label=\"" << pass.name << "\"";
        if (pass.culled) {
            ss << ", style=dashed, color=gray";
        }
        ss << "];\n";
    }

    ss << "\n";

    // 输出依赖边
    for (uint32_t i = 0; i < m_passes.size(); i++) {
        for (uint32_t dep : m_passes[i].dependencies) {
            ss << "    pass_" << dep << " -> pass_" << i << ";\n";
        }
    }

    ss << "}\n";
    return ss.str();
}

void RenderGraph::Reset() {
    m_textures.clear();
    m_buffers.clear();
    m_passes.clear();
    m_executionOrder.clear();
    m_textureNameMap.clear();
    m_bufferNameMap.clear();
    m_compiled = false;
    m_stats = {};
}

// ============ Context ============

RGContext::RGContext(RenderGraph& graph) : m_graph(graph) {}

Texture* RGContext::GetTexture(RGTextureHandle handle) {
    if (!handle.IsValid()) return nullptr;
    return m_graph.m_textures[handle.index].physicalTexture.get();
}

} // namespace Mint
```

---

# 5. 使用示例与最佳实践

## 5.1 基础延迟渲染管线

```cpp
void SetupDeferredPipeline(RenderGraph& graph, Scene& scene, Camera& camera) {
    auto& viewport = camera.GetViewport();

    // ========== 创建资源 ==========

    // GBuffer
    auto gbufferAlbedo = graph.CreateTexture("GBuffer_Albedo", {
        .width = viewport.width,
        .height = viewport.height,
        .format = RGTextureFormat::RGBA8
    });

    auto gbufferNormal = graph.CreateTexture("GBuffer_Normal", {
        .width = viewport.width,
        .height = viewport.height,
        .format = RGTextureFormat::RGBA16F  // 世界空间法线
    });

    auto gbufferMaterial = graph.CreateTexture("GBuffer_Material", {
        .width = viewport.width,
        .height = viewport.height,
        .format = RGTextureFormat::RGBA8    // Roughness, Metallic, AO, ...
    });

    auto depthBuffer = graph.CreateTexture("DepthBuffer", {
        .width = viewport.width,
        .height = viewport.height,
        .format = RGTextureFormat::Depth24Stencil8
    });

    // 阴影贴图
    auto shadowMap = graph.CreateTexture("ShadowMap", {
        .width = 4096,
        .height = 4096,
        .format = RGTextureFormat::Depth32F
    });

    // HDR 缓冲
    auto hdrBuffer = graph.CreateTexture("HDR", {
        .width = viewport.width,
        .height = viewport.height,
        .format = RGTextureFormat::RGBA16F
    });

    // 最终输出
    auto finalOutput = graph.ImportTexture("Backbuffer", GetSwapchainTexture());

    // ========== 注册 Pass ==========

    // 1. Shadow Pass
    graph.AddPass("ShadowPass",
        [&](RGPassBuilder& builder) {
            builder.Write(shadowMap, RGWriteAccess::DepthStencil);
        },
        [&scene](RGContext& ctx) {
            auto* shadow = ctx.GetTexture(shadowMap);

            ctx.SetRenderTarget({}, shadowMap);
            ctx.SetViewport(0, 0, 4096, 4096);

            // 绑定阴影 Shader
            shadowShader->Bind();

            // 渲染阴影投射物
            for (auto& mesh : scene.GetShadowCasters()) {
                mesh.Draw();
            }
        }
    );

    // 2. GBuffer Pass
    graph.AddPass("GBufferPass",
        [&](RGPassBuilder& builder) {
            builder.Write(gbufferAlbedo, RGWriteAccess::RenderTarget);
            builder.Write(gbufferNormal, RGWriteAccess::RenderTarget);
            builder.Write(gbufferMaterial, RGWriteAccess::RenderTarget);
            builder.Write(depthBuffer, RGWriteAccess::DepthStencil);
        },
        [&scene, &camera, &viewport](RGContext& ctx) {
            ctx.SetRenderTarget(
                {gbufferAlbedo, gbufferNormal, gbufferMaterial},
                depthBuffer
            );
            ctx.SetViewport(0, 0, viewport.width, viewport.height);
            ctx.Clear();

            gbufferShader->Bind();
            gbufferShader->SetMat4("u_ViewProjection", camera.GetViewProjection());

            for (auto& mesh : scene.GetMeshes()) {
                mesh.Draw();
            }
        }
    );

    // 3. Lighting Pass
    graph.AddPass("LightingPass",
        [&](RGPassBuilder& builder) {
            builder.Read(gbufferAlbedo);
            builder.Read(gbufferNormal);
            builder.Read(gbufferMaterial);
            builder.Read(depthBuffer);
            builder.Read(shadowMap);
            builder.Write(hdrBuffer, RGWriteAccess::RenderTarget);
        },
        [&scene, &camera](RGContext& ctx) {
            ctx.SetRenderTarget(hdrBuffer, {});

            lightingShader->Bind();
            ctx.BindTexture(0, gbufferAlbedo);
            ctx.BindTexture(1, gbufferNormal);
            ctx.BindTexture(2, gbufferMaterial);
            ctx.BindTexture(3, depthBuffer);
            ctx.BindTexture(4, shadowMap);

            // 设置光源数据
            lightingShader->SetVec3("u_CameraPos", camera.GetPosition());
            UploadLightData(lightingShader, scene.GetLights());

            ctx.DrawFullscreenQuad();
        }
    );

    // 4. Tone Mapping + Output
    graph.AddPass("ToneMapping",
        [&](RGPassBuilder& builder) {
            builder.Read(hdrBuffer);
            builder.Write(finalOutput, RGWriteAccess::RenderTarget);
        },
        [](RGContext& ctx) {
            ctx.SetRenderTarget(finalOutput, {});

            tonemapShader->Bind();
            ctx.BindTexture(0, hdrBuffer);

            ctx.DrawFullscreenQuad();
        }
    );
}

// 主循环
void Renderer::RenderFrame(Scene& scene, Camera& camera) {
    m_renderGraph.Reset();  // 清除上一帧数据

    SetupDeferredPipeline(m_renderGraph, scene, camera);

    m_renderGraph.Compile();
    m_renderGraph.Execute();
}
```

## 5.2 添加后处理效果

```cpp
void AddBloomPass(RenderGraph& graph, RGTextureHandle input, RGTextureHandle output) {
    auto& inputDesc = graph.GetTextureDesc(input);

    // Bloom 需要多个中间纹理
    auto brightPass = graph.CreateTexture("Bloom_Bright", {
        .width = inputDesc.width / 2,
        .height = inputDesc.height / 2,
        .format = RGTextureFormat::RGBA16F
    });

    auto blurH = graph.CreateTexture("Bloom_BlurH", {
        .width = inputDesc.width / 2,
        .height = inputDesc.height / 2,
        .format = RGTextureFormat::RGBA16F
    });

    auto blurV = graph.CreateTexture("Bloom_BlurV", {
        .width = inputDesc.width / 2,
        .height = inputDesc.height / 2,
        .format = RGTextureFormat::RGBA16F
    });

    // 1. 提取高亮
    graph.AddPass("Bloom_BrightPass",
        [&](RGPassBuilder& b) {
            b.Read(input);
            b.Write(brightPass);
        },
        [=](RGContext& ctx) {
            brightExtractShader->Bind();
            ctx.BindTexture(0, input);
            ctx.SetRenderTarget(brightPass, {});
            ctx.DrawFullscreenQuad();
        }
    );

    // 2. 水平模糊
    graph.AddPass("Bloom_BlurH",
        [&](RGPassBuilder& b) {
            b.Read(brightPass);
            b.Write(blurH);
        },
        [=](RGContext& ctx) {
            gaussianBlurShader->Bind();
            gaussianBlurShader->SetVec2("u_Direction", {1.0f, 0.0f});
            ctx.BindTexture(0, brightPass);
            ctx.SetRenderTarget(blurH, {});
            ctx.DrawFullscreenQuad();
        }
    );

    // 3. 垂直模糊
    graph.AddPass("Bloom_BlurV",
        [&](RGPassBuilder& b) {
            b.Read(blurH);
            b.Write(blurV);
        },
        [=](RGContext& ctx) {
            gaussianBlurShader->Bind();
            gaussianBlurShader->SetVec2("u_Direction", {0.0f, 1.0f});
            ctx.BindTexture(0, blurH);
            ctx.SetRenderTarget(blurV, {});
            ctx.DrawFullscreenQuad();
        }
    );

    // 4. 合成
    graph.AddPass("Bloom_Composite",
        [&](RGPassBuilder& b) {
            b.Read(input);
            b.Read(blurV);
            b.Write(output);
        },
        [=](RGContext& ctx) {
            bloomCompositeShader->Bind();
            ctx.BindTexture(0, input);
            ctx.BindTexture(1, blurV);
            ctx.SetRenderTarget(output, {});
            ctx.DrawFullscreenQuad();
        }
    );
}
```

## 5.3 条件 Pass

```cpp
void SetupConditionalPasses(RenderGraph& graph, const RenderSettings& settings) {
    auto hdrBuffer = graph.CreateTexture("HDR", hdrDesc);
    auto finalBuffer = graph.ImportTexture("Final", backbuffer);

    RGTextureHandle currentInput = hdrBuffer;

    // 根据设置条件添加后处理
    if (settings.enableBloom) {
        auto bloomOutput = graph.CreateTexture("BloomOutput", hdrDesc);
        AddBloomPass(graph, currentInput, bloomOutput);
        currentInput = bloomOutput;
    }

    if (settings.enableSSAO) {
        auto ssaoOutput = graph.CreateTexture("SSAOOutput", hdrDesc);
        AddSSAOPass(graph, currentInput, ssaoOutput);
        currentInput = ssaoOutput;
    }

    if (settings.enableFXAA) {
        auto fxaaOutput = graph.CreateTexture("FXAAOutput", ldrDesc);
        AddFXAAPass(graph, currentInput, fxaaOutput);
        currentInput = fxaaOutput;
    }

    // 最终输出
    AddFinalPass(graph, currentInput, finalBuffer);
}
```

---

# 6. 开发规划与里程碑

## 6.1 实现路线图

```
┌─────────────────────────────────────────────────────────────────────────┐
│                     RenderGraph 开发路线图                               │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                          │
│  Phase 1: 基础框架                                                       │
│  ════════════════                                                        │
│  ├── RGHandle, RGTextureDesc, RGBufferDesc                              │
│  ├── RenderGraph 主类框架                                                │
│  ├── CreateTexture/ImportTexture                                        │
│  ├── AddPass 基本实现                                                    │
│  └── 简单线性执行（无依赖分析）                                          │
│                                                                          │
│  验证点: 能够注册 Pass 并按顺序执行                                      │
│                                                                          │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                          │
│  Phase 2: 依赖分析                                                       │
│  ════════════════                                                        │
│  ├── BuildDependencyGraph 实现                                          │
│  ├── TopologicalSort 拓扑排序                                           │
│  ├── 循环依赖检测                                                        │
│  └── ExportGraphviz 调试输出                                            │
│                                                                          │
│  验证点: 正确的执行顺序，Graphviz 可视化                                 │
│                                                                          │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                          │
│  Phase 3: 资源生命周期                                                   │
│  ════════════════════                                                    │
│  ├── CalculateResourceLifetimes                                         │
│  ├── 延迟资源创建                                                        │
│  ├── 自动资源释放（返回池）                                              │
│  └── 内存使用统计                                                        │
│                                                                          │
│  验证点: 资源在正确时机创建/释放                                         │
│                                                                          │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                          │
│  Phase 4: Pass 裁剪                                                      │
│  ══════════════════                                                      │
│  ├── 标记输出资源（final output）                                        │
│  ├── 反向遍历标记有效 Pass                                               │
│  └── 跳过 culled Pass                                                   │
│                                                                          │
│  验证点: 未使用的 Pass 被正确跳过                                        │
│                                                                          │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                          │
│  Phase 5: 资源别名（高级）                                               │
│  ══════════════════════════                                              │
│  ├── 生命周期重叠检测                                                    │
│  ├── 内存块分配算法                                                      │
│  ├── 物理资源复用                                                        │
│  └── 别名统计与调试                                                      │
│                                                                          │
│  验证点: 内存使用明显减少                                                │
│                                                                          │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                          │
│  Phase 6: 同步与屏障（Vulkan/DX12）                                      │
│  ════════════════════════════════════                                    │
│  ├── ResourceState 追踪                                                  │
│  ├── 自动屏障插入                                                        │
│  ├── 异步 Compute 支持                                                   │
│  └── 多队列执行                                                          │
│                                                                          │
│  验证点: Vulkan 验证层无错误                                             │
│                                                                          │
└─────────────────────────────────────────────────────────────────────────┘
```

## 6.2 各阶段详细任务

### Phase 1: 基础框架

```
任务清单:
□ 创建 rg_types.h - 定义基础类型
□ 创建 rg_resource.h - 资源结构定义
□ 创建 rg_pass.h - Pass 相关类
□ 创建 render_graph.h/cpp - 主类
□ 实现 CreateTexture/CreateBuffer
□ 实现 ImportTexture/ImportBuffer
□ 实现 AddPass 模板方法
□ 实现简单线性 Execute

验证方法:
- 创建简单的 2-Pass 管线
- 确认 Pass 按添加顺序执行
- 确认资源正确创建
```

### Phase 2: 依赖分析

```
任务清单:
□ 在 RGTexture 中添加 lastWriter 追踪
□ 实现 BuildDependencyGraph
□ 实现 TopologicalSort (Kahn 算法)
□ 添加循环依赖检测和错误报告
□ 实现 ExportGraphviz
□ 集成到 Compile 流程

验证方法:
- 创建有依赖关系的 Pass
- 故意打乱添加顺序
- 确认执行顺序正确
- 用 Graphviz 可视化验证
```

### Phase 3: 资源生命周期

```
任务清单:
□ 添加 firstUsePass/lastUsePass 字段
□ 实现 CalculateResourceLifetimes
□ 修改 Execute 为延迟创建资源
□ 在资源生命周期结束时标记可回收
□ 实现 PrintStatistics

验证方法:
- 观察资源创建时机
- 确认瞬态资源在合适时机创建
- 对比内存使用统计
```

## 6.3 里程碑检查点

| 里程碑 | 完成标志 | 验证方法 |
|--------|----------|----------|
| M1: 基础可用 | Pass 能执行 | 单 Pass 渲染三角形 |
| M2: 依赖正确 | 自动排序 | 多 Pass 延迟渲染 |
| M3: 生命周期 | 按需创建 | 内存使用对比 |
| M4: 裁剪有效 | 无用 Pass 跳过 | 添加未连接 Pass |
| M5: 别名优化 | 内存复用 | 复杂管线内存减少 |
| M6: 同步正确 | 无验证错误 | Vulkan 运行 |

---

# 7. 进阶主题

## 7.1 异步计算

```cpp
// 标记 Pass 为 Compute 队列
graph.AddPass("AsyncSSAO",
    [&](RGPassBuilder& builder) {
        builder.SetQueue(RGQueue::Compute);  // 异步计算
        builder.Read(depthBuffer);
        builder.Write(ssaoBuffer, RGWriteAccess::UnorderedAccess);
    },
    [](RGContext& ctx) {
        ssaoComputeShader->Bind();
        ctx.BindTexture(0, depthBuffer);
        ctx.BindUAV(0, ssaoBuffer);
        ctx.Dispatch(width / 8, height / 8, 1);
    }
);

// 框架自动在 Graphics 和 Compute 之间插入同步
```

## 7.2 多帧资源

```cpp
// 历史帧资源（用于 TAA、运动模糊等）
class TemporalRenderGraph {
    RGTextureHandle m_historyBuffer[2];  // 双缓冲
    uint32_t m_currentFrame = 0;

    void Setup(RenderGraph& graph) {
        uint32_t current = m_currentFrame % 2;
        uint32_t previous = (m_currentFrame + 1) % 2;

        // 当前帧写入
        auto currentBuffer = graph.ImportTexture("Current", m_historyBuffer[current]);

        // 上一帧读取
        auto previousBuffer = graph.ImportTexture("Previous", m_historyBuffer[previous]);

        graph.AddPass("TAA", [&](auto& b) {
            b.Read(previousBuffer);
            b.Write(currentBuffer);
        }, ...);

        m_currentFrame++;
    }
};
```

## 7.3 调试可视化

```cpp
// 集成 ImGui 的 RenderGraph 可视化
void RenderGraphDebugWindow(const RenderGraph& graph) {
    ImGui::Begin("RenderGraph Debug");

    // 显示执行顺序
    if (ImGui::CollapsingHeader("Execution Order")) {
        for (uint32_t i = 0; i < graph.GetExecutionOrder().size(); i++) {
            auto& pass = graph.GetPass(graph.GetExecutionOrder()[i]);
            ImGui::Text("%d. %s %s", i, pass.name.c_str(),
                        pass.culled ? "(culled)" : "");
        }
    }

    // 显示资源统计
    if (ImGui::CollapsingHeader("Resources")) {
        auto& stats = graph.GetStatistics();
        ImGui::Text("Total Memory: %.2f MB", stats.totalTextureMemory / 1024.0f / 1024.0f);
        ImGui::Text("Aliased Saved: %.2f MB", stats.aliasedMemorySaved / 1024.0f / 1024.0f);
        ImGui::Text("Pass Count: %d (%d culled)", stats.passCount, stats.culledPassCount);
    }

    // 导出 Graphviz
    if (ImGui::Button("Export Graphviz")) {
        std::string dot = graph.ExportGraphviz();
        // 保存到文件或复制到剪贴板
    }

    ImGui::End();
}
```

---

# 8. 参考资源

## 8.1 必读资料

### GDC 演讲
- **FrameGraph: Extensible Rendering Architecture in Frostbite** (GDC 2017)
  - Frostbite 引擎的 RenderGraph 设计，最经典的参考
  - [PDF](https://www.gdcvault.com/play/1024612/FrameGraph-Extensible-Rendering-Architecture-in)

- **Halcyon Architecture** (GDC 2019)
  - EA 新引擎的渲染架构演进

### 博客文章
- **Render Graphs and Vulkan — a deep dive** - Hans-Kristian Arntzen
  - 深入探讨 RenderGraph 与 Vulkan 集成

- **High-Level Rendering Using Render Graphs** - Our Machinery
  - 实用的实现指南

## 8.2 开源实现参考

| 项目 | 语言 | 特点 |
|------|------|------|
| **Granite** | C++ | Hans-Kristian Arntzen 的 Vulkan 引擎，RenderGraph 实现清晰 |
| **Filament** | C++ | Google 的移动端渲染引擎，有 FrameGraph |
| **wgpu** | Rust | WebGPU 实现，有简洁的 RenderGraph |
| **Godot 4** | C++ | RenderingDevice + FrameGraph |

## 8.3 书籍

- **Real-Time Rendering, 4th Edition** - Chapter 23: Render Pipelines
- **GPU Gems 3** - 部分章节涉及渲染管线优化

## 8.4 相关概念

理解 RenderGraph 需要的前置知识：
- 图论基础（DAG、拓扑排序）
- GPU 资源管理（纹理、缓冲区）
- 渲染管线状态（混合、深度测试）
- 同步原语（屏障、信号量）- 仅现代 API

---

# 附录 A: 术语表

| 术语 | 英文 | 含义 |
|------|------|------|
| 渲染图 | RenderGraph | 声明式渲染管线描述系统 |
| 帧图 | FrameGraph | 同 RenderGraph（Frostbite 术语） |
| 瞬态资源 | Transient Resource | 仅在当前帧存在的资源 |
| 导入资源 | Imported Resource | 外部管理生命周期的资源 |
| 资源别名 | Resource Aliasing | 多个虚拟资源共享物理内存 |
| 拓扑排序 | Topological Sort | DAG 的线性排序算法 |
| 屏障 | Barrier | GPU 资源状态转换同步点 |
| 裁剪 | Culling | 跳过输出未被使用的 Pass |

---

# 附录 B: 常见问题

## Q1: RenderGraph 适合所有项目吗？

**A**: 不一定。对于简单的前向渲染管线，传统方式可能更直接。RenderGraph 的优势在复杂管线（延迟渲染、多后处理、可配置管线）中体现明显。

## Q2: 每帧都要重建 RenderGraph 吗？

**A**: 是的，这是设计意图。好处是：
- 动态适应场景变化
- 自动处理资源失效
- 简化状态管理

对于静态管线，可以缓存 Compile 结果。

## Q3: RenderGraph 的性能开销？

**A**: 主要开销在 Compile 阶段（拓扑排序、资源分配）。对于典型的 10-30 个 Pass，这个开销通常小于 0.1ms，远小于实际渲染时间。

## Q4: 如何调试 RenderGraph？

**A**:
1. 使用 `ExportGraphviz()` 可视化依赖图
2. 添加 Pass 名称和资源名称便于追踪
3. 使用 GPU 调试工具（RenderDoc、PIX）
4. 实现 `PrintStatistics()` 监控资源使用

---

*文档版本: 1.0*
*最后更新: 2026-01-13*
