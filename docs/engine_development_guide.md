# MintEngine 引擎开发指南

> 本文档从引擎开发和学习的角度，详细阐述各模块的设计思路、设计模式选择理由、模块间关系，以及代码示例。

---

## 目录

1. [渲染架构总览](#1-渲染架构总览)
2. [RenderPass 系统设计](#2-renderpass-系统设计)
3. [Shader 系统与反射机制](#3-shader-系统与反射机制)
4. [UniformBuffer 与 GPU 数据管理](#4-uniformbuffer-与-gpu-数据管理)
5. [Material 系统设计](#5-material-系统设计)
6. [Mesh 系统架构](#6-mesh-系统架构)
7. [资源管理系统](#7-资源管理系统)
8. [场景与 ECS 系统](#8-场景与-ecs-系统)
9. [开发阶段规划](#9-开发阶段规划)

---

## 1. 渲染架构总览

### 1.1 现代渲染引擎的分层架构

```
┌─────────────────────────────────────────────────────────────┐
│                      Application Layer                       │
│                   (Game Logic, Editor)                       │
├─────────────────────────────────────────────────────────────┤
│                      Scene Layer                             │
│            (Entity, Component, SceneGraph)                   │
├─────────────────────────────────────────────────────────────┤
│                     Renderer Layer                           │
│     (RenderGraph, RenderPass, Material, Mesh, Light)        │
├─────────────────────────────────────────────────────────────┤
│                        RHI Layer                             │
│  (RHI Abstraction: Buffer, Texture, Shader, Pipeline)       │
├─────────────────────────────────────────────────────────────┤
│                   Graphics API Layer                         │
│              (OpenGL, Vulkan, D3D12, Metal)                 │
└─────────────────────────────────────────────────────────────┘
```

### 1.2 你当前架构的分析

你的 `RenderSystem` 目前混合了多个层次的职责：

```cpp
// 当前设计 - RenderSystem 承担了太多职责
class RenderSystem {
    RenderCommandQueue m_commandQueue;     // RHI层：命令队列
    ShaderLibrary m_shaderLibrary;         // 资源层：着色器管理
    RendererAPI m_rendererAPI;             // RHI层：API抽象
    SceneData* m_sceneData;                // 场景层：场景数据
};
```

### 1.3 建议的架构重构

**核心思想**：职责分离 + 数据驱动

```cpp
// 推荐架构

// 1. RHI Layer - 硬件抽象
namespace RHI {
    class Device;           // GPU设备抽象
    class CommandBuffer;    // 命令缓冲
    class Pipeline;         // 渲染管线状态
    class Buffer;           // GPU缓冲
    class Texture;          // 纹理
    class Shader;           // 着色器程序
}

// 2. Renderer Layer - 渲染逻辑
class Renderer {
    void BeginFrame();
    void EndFrame();
    void RenderScene(Scene& scene, Camera& camera);

private:
    RenderGraph m_renderGraph;      // 渲染图
    std::vector<RenderPass*> m_passes;
};

// 3. 具体的Pass实现
class ForwardPass : public RenderPass { };
class ShadowPass : public RenderPass { };
class SkyboxPass : public RenderPass { };
class PostProcessPass : public RenderPass { };
```

### 1.4 为什么采用这种分层？

| 层次 | 职责 | 变化频率 | 示例 |
|------|------|----------|------|
| Application | 游戏逻辑 | 高 | 玩家输入、AI |
| Scene | 场景组织 | 中 | Entity创建、组件更新 |
| Renderer | 渲染策略 | 低 | Forward/Deferred切换 |
| RHI | 硬件抽象 | 很低 | OpenGL→Vulkan迁移 |

**设计原则**：高层依赖低层，低层不知道高层存在。这样当你添加Vulkan后端时，只需要实现RHI层，上层代码无需修改。

---

## 2. RenderPass 系统设计

### 2.1 什么是 RenderPass？

RenderPass 代表一次完整的渲染过程，包括：
- **输入**：需要渲染的数据（Mesh、Material、Transform）
- **输出**：渲染目标（Framebuffer 的 Color/Depth Attachment）
- **状态**：渲染管线状态（Blend、DepthTest、Cull）

### 2.2 RenderPass 应该何时引入？

**建议时机**：在 Framebuffer 和 Material 系统完成后立即引入

**原因**：
1. RenderPass 需要 Framebuffer 作为渲染目标
2. RenderPass 需要 Material 来确定渲染状态
3. 提前引入可以指导其他系统的设计

### 2.3 RenderPass 的两种设计模式

#### 模式 A：继承式（简单，适合学习）

```cpp
// 基类定义渲染流程的骨架
class RenderPass : public RefCounter {
public:
    virtual ~RenderPass() = default;

    // Template Method Pattern - 定义渲染流程骨架
    void Execute(const RenderContext& context) {
        Begin(context);
        Render(context);
        End(context);
    }

protected:
    virtual void Begin(const RenderContext& context) {
        m_framebuffer->Bind();
        // 设置视口、清除缓冲等
    }

    virtual void Render(const RenderContext& context) = 0;  // 子类必须实现

    virtual void End(const RenderContext& context) {
        m_framebuffer->Unbind();
    }

protected:
    Ref<Framebuffer> m_framebuffer;
    RenderPassSpecification m_specification;
};

// 具体实现
class ForwardPass : public RenderPass {
protected:
    void Render(const RenderContext& context) override {
        for (auto& drawable : context.drawList) {
            drawable.material->Bind();
            drawable.mesh->Draw();
        }
    }
};

class ShadowPass : public RenderPass {
protected:
    void Render(const RenderContext& context) override {
        // 只渲染深度，使用简单的depth shader
        m_depthShader->Bind();
        for (auto& drawable : context.shadowCasters) {
            drawable.mesh->Draw();
        }
    }
};
```

**优点**：直观、易于理解、每个Pass独立
**缺点**：Pass之间的依赖关系不明确

#### 模式 B：RenderGraph 数据驱动式（复杂，工业级）

```cpp
// RenderGraph - 描述Pass之间的依赖关系
class RenderGraph {
public:
    // 声明式API - 描述"要什么"而不是"怎么做"
    void AddPass(const std::string& name,
                 std::function<void(RenderGraphBuilder&)> setup,
                 std::function<void(const RenderContext&)> execute);

    void Compile();   // 分析依赖、裁剪无用Pass
    void Execute();   // 按拓扑序执行

private:
    std::vector<RenderGraphNode> m_nodes;
    std::vector<RenderGraphEdge> m_edges;
};

// 使用示例
void Renderer::BuildRenderGraph() {
    m_graph.AddPass("ShadowPass",
        [](RenderGraphBuilder& builder) {
            // 声明输出
            builder.CreateTexture("ShadowMap", {2048, 2048, Format::Depth});
        },
        [this](const RenderContext& ctx) {
            // 实际渲染逻辑
            RenderShadowMap(ctx);
        }
    );

    m_graph.AddPass("ForwardPass",
        [](RenderGraphBuilder& builder) {
            // 声明输入依赖
            builder.Read("ShadowMap");
            // 声明输出
            builder.Write("SceneColor");
            builder.Write("SceneDepth");
        },
        [this](const RenderContext& ctx) {
            RenderForward(ctx);
        }
    );

    m_graph.AddPass("PostProcess",
        [](RenderGraphBuilder& builder) {
            builder.Read("SceneColor");
            builder.Write("FinalOutput");
        },
        [this](const RenderContext& ctx) {
            ApplyPostProcess(ctx);
        }
    );

    m_graph.Compile();  // 自动分析依赖关系
}
```

**优点**：自动依赖分析、资源生命周期管理、可优化
**缺点**：实现复杂、需要更多基础设施

### 2.4 建议的开发路径

```
阶段1: 基础 RenderPass (现在)
├── 实现简单的 ForwardPass
├── 支持单一 Framebuffer 输出
└── 手动管理 Pass 顺序

阶段2: 多 Pass 渲染 (Material 之后)
├── ShadowPass
├── SkyboxPass
├── 简单的 Pass 依赖管理
└── Pass 之间传递纹理

阶段3: RenderGraph (可选，高级)
├── 自动依赖分析
├── 资源自动管理
└── Pass 裁剪优化
```

### 2.5 RenderPass Specification 设计

```cpp
// 配置驱动的 RenderPass 创建
struct RenderPassSpecification {
    std::string name;

    // 渲染目标配置
    struct AttachmentSpec {
        TextureFormat format;
        LoadOp loadOp = LoadOp::Clear;    // Clear/Load/DontCare
        StoreOp storeOp = StoreOp::Store; // Store/DontCare
        glm::vec4 clearColor = {0, 0, 0, 1};
        float clearDepth = 1.0f;
    };
    std::vector<AttachmentSpec> colorAttachments;
    std::optional<AttachmentSpec> depthAttachment;

    // 视口配置
    uint32_t width = 0;   // 0 表示使用窗口大小
    uint32_t height = 0;

    // 渲染状态默认值
    bool depthTest = true;
    bool depthWrite = true;
    CullMode cullMode = CullMode::Back;
};

// 工厂创建
Ref<RenderPass> RenderPass::Create(const RenderPassSpecification& spec) {
    return Ref<RenderPass>::Create(new RenderPass(spec));
}
```

---

## 3. Shader 系统与反射机制

### 3.1 当前实现分析

你的 Shader 反射已经有了基础框架：

```cpp
// 当前实现
class ShaderUniform {
    std::string m_name;
    ShaderUniformType m_type;
    uint32_t m_size;
    uint32_t m_offset;
};

class ShaderBuffer {  // 用于 UBO
    std::string m_name;
    uint32_t m_size;
    std::unordered_map<std::string, ShaderUniform> m_uniforms;
};
```

### 3.2 反射系统的完整设计

```cpp
// ============ 着色器资源类型 ============

enum class ShaderResourceType {
    None = 0,
    UniformBuffer,      // UBO
    StorageBuffer,      // SSBO (Compute Shader)
    Texture2D,
    TextureCube,
    Sampler,
    PushConstant,       // Vulkan
};

// 着色器阶段
enum class ShaderStage : uint32_t {
    Vertex   = BIT(0),
    Fragment = BIT(1),
    Geometry = BIT(2),
    Compute  = BIT(3),
    All      = Vertex | Fragment | Geometry | Compute
};

// ============ 反射数据结构 ============

// 单个Uniform成员
struct ShaderUniform {
    std::string name;
    ShaderUniformType type;
    uint32_t size;
    uint32_t offset;        // 在Buffer中的偏移
    uint32_t arraySize = 1; // 数组大小，1表示非数组
};

// Uniform Buffer 描述
struct ShaderUniformBuffer {
    std::string name;
    uint32_t size;
    uint32_t bindingPoint;
    ShaderStage stage;      // 哪个阶段使用
    std::vector<ShaderUniform> members;

    // 辅助方法
    const ShaderUniform* FindMember(const std::string& name) const;
};

// 纹理/采样器资源
struct ShaderResource {
    std::string name;
    ShaderResourceType type;
    uint32_t bindingPoint;
    uint32_t arraySize = 1;
    ShaderStage stage;
};

// ============ 完整的反射数据 ============

struct ShaderReflectionData {
    // Uniform Buffers (UBO)
    std::vector<ShaderUniformBuffer> uniformBuffers;

    // 独立的 Uniforms (非UBO)
    std::vector<ShaderUniform> uniforms;

    // 纹理资源
    std::vector<ShaderResource> resources;

    // Push Constants (Vulkan用)
    std::vector<ShaderUniform> pushConstants;

    // 辅助查询
    const ShaderUniformBuffer* FindUniformBuffer(const std::string& name) const;
    const ShaderResource* FindResource(const std::string& name) const;
    int32_t FindUniformLocation(const std::string& name) const;
};
```

### 3.3 OpenGL 反射实现

```cpp
void OpenGLShader::Reflect() {
    // ============ 1. 反射 Uniform Buffers ============
    GLint numUBOs = 0;
    glGetProgramiv(m_programId, GL_ACTIVE_UNIFORM_BLOCKS, &numUBOs);

    for (GLint i = 0; i < numUBOs; i++) {
        GLint nameLength;
        glGetActiveUniformBlockiv(m_programId, i, GL_UNIFORM_BLOCK_NAME_LENGTH, &nameLength);

        std::string name(nameLength - 1, '\0');
        glGetActiveUniformBlockName(m_programId, i, nameLength, nullptr, name.data());

        GLint dataSize;
        glGetActiveUniformBlockiv(m_programId, i, GL_UNIFORM_BLOCK_DATA_SIZE, &dataSize);

        GLint bindingPoint;
        glGetActiveUniformBlockiv(m_programId, i, GL_UNIFORM_BLOCK_BINDING, &bindingPoint);

        // 获取 Block 中的 Uniform 数量
        GLint numUniforms;
        glGetActiveUniformBlockiv(m_programId, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &numUniforms);

        // 获取 Uniform 索引
        std::vector<GLint> uniformIndices(numUniforms);
        glGetActiveUniformBlockiv(m_programId, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES,
                                   uniformIndices.data());

        ShaderUniformBuffer ubo;
        ubo.name = name;
        ubo.size = dataSize;
        ubo.bindingPoint = bindingPoint;

        // 解析每个成员
        for (GLint uniformIndex : uniformIndices) {
            GLint uniformNameLength;
            glGetActiveUniformsiv(m_programId, 1, (GLuint*)&uniformIndex,
                                   GL_UNIFORM_NAME_LENGTH, &uniformNameLength);

            std::string uniformName(uniformNameLength - 1, '\0');
            GLint size;
            GLenum type;
            glGetActiveUniform(m_programId, uniformIndex, uniformNameLength,
                               nullptr, &size, &type, uniformName.data());

            GLint offset;
            glGetActiveUniformsiv(m_programId, 1, (GLuint*)&uniformIndex,
                                   GL_UNIFORM_OFFSET, &offset);

            ShaderUniform member;
            member.name = uniformName;
            member.type = GLTypeToShaderUniformType(type);
            member.size = ShaderUniformTypeSize(member.type);
            member.offset = offset;
            member.arraySize = size;

            ubo.members.push_back(member);
        }

        m_reflectionData.uniformBuffers.push_back(ubo);
    }

    // ============ 2. 反射独立 Uniforms ============
    GLint numUniforms;
    glGetProgramiv(m_programId, GL_ACTIVE_UNIFORMS, &numUniforms);

    for (GLint i = 0; i < numUniforms; i++) {
        // 检查是否属于某个 UBO
        GLint blockIndex;
        glGetActiveUniformsiv(m_programId, 1, (GLuint*)&i,
                               GL_UNIFORM_BLOCK_INDEX, &blockIndex);

        if (blockIndex != -1) continue;  // 属于 UBO，跳过

        char name[256];
        GLint size;
        GLenum type;
        glGetActiveUniform(m_programId, i, sizeof(name), nullptr, &size, &type, name);

        if (IsTextureType(type)) {
            // 纹理资源
            ShaderResource resource;
            resource.name = name;
            resource.type = GLTypeToResourceType(type);
            resource.bindingPoint = glGetUniformLocation(m_programId, name);
            resource.arraySize = size;
            m_reflectionData.resources.push_back(resource);
        } else {
            // 普通 Uniform
            ShaderUniform uniform;
            uniform.name = name;
            uniform.type = GLTypeToShaderUniformType(type);
            uniform.size = ShaderUniformTypeSize(uniform.type);
            uniform.offset = 0;
            uniform.arraySize = size;
            m_reflectionData.uniforms.push_back(uniform);
        }
    }

    // ============ 3. 缓存 Uniform Locations ============
    for (const auto& uniform : m_reflectionData.uniforms) {
        m_uniformLocationCache[uniform.name] =
            glGetUniformLocation(m_programId, uniform.name.c_str());
    }
}
```

### 3.4 反射的应用场景

```cpp
// 场景1: Material 自动生成
Ref<Material> Material::CreateFromShader(const Ref<Shader>& shader) {
    auto material = Ref<Material>::Create();
    material->m_shader = shader;

    const auto& reflection = shader->GetReflectionData();

    // 根据反射数据分配 Uniform 存储
    uint32_t totalSize = 0;
    for (const auto& uniform : reflection.uniforms) {
        totalSize += uniform.size * uniform.arraySize;
    }
    material->m_uniformStorage.Allocate(totalSize);

    // 为纹理槽位预留
    for (const auto& resource : reflection.resources) {
        if (resource.type == ShaderResourceType::Texture2D) {
            material->m_textureSlots[resource.name] = nullptr;
        }
    }

    return material;
}

// 场景2: 编辑器自动生成 UI
void MaterialEditor::DrawProperties(Material& material) {
    const auto& reflection = material.GetShader()->GetReflectionData();

    for (const auto& uniform : reflection.uniforms) {
        switch (uniform.type) {
            case ShaderUniformType::Float:
                DrawFloatControl(uniform.name, material.Get<float>(uniform.name));
                break;
            case ShaderUniformType::Vec3:
                DrawVec3Control(uniform.name, material.Get<glm::vec3>(uniform.name));
                break;
            case ShaderUniformType::Vec4:
                DrawColorControl(uniform.name, material.Get<glm::vec4>(uniform.name));
                break;
            // ...
        }
    }
}

// 场景3: 序列化
void MaterialSerializer::Serialize(const Material& material, YAML::Emitter& out) {
    const auto& reflection = material.GetShader()->GetReflectionData();

    out << YAML::Key << "Properties" << YAML::Value << YAML::BeginMap;
    for (const auto& uniform : reflection.uniforms) {
        out << YAML::Key << uniform.name;
        // 根据类型序列化值
        SerializeUniformValue(out, uniform.type, material.GetRaw(uniform.name));
    }
    out << YAML::EndMap;
}
```

### 3.5 着色器预处理与包含系统

```cpp
class ShaderPreprocessor {
public:
    // 处理 #include 指令
    std::string Process(const std::string& source, const std::filesystem::path& basePath) {
        std::string result;
        std::istringstream stream(source);
        std::string line;

        while (std::getline(stream, line)) {
            if (line.find("#include") != std::string::npos) {
                // 解析包含路径
                auto includePath = ParseIncludePath(line);
                auto fullPath = basePath / includePath;

                // 防止循环包含
                if (m_includedFiles.find(fullPath) != m_includedFiles.end()) {
                    continue;
                }
                m_includedFiles.insert(fullPath);

                // 递归处理
                auto includeSource = ReadFile(fullPath);
                result += Process(includeSource, fullPath.parent_path());
            } else {
                result += line + "\n";
            }
        }

        return result;
    }

private:
    std::set<std::filesystem::path> m_includedFiles;
};

// 常用的 Shader 包含文件
// shaders/common/camera.glsl
/*
layout(std140, binding = 0) uniform CameraData {
    mat4 u_ViewProjection;
    mat4 u_View;
    mat4 u_Projection;
    vec3 u_CameraPosition;
    float u_NearPlane;
    float u_FarPlane;
};
*/

// shaders/common/lighting.glsl
/*
struct DirectionalLight {
    vec3 direction;
    vec3 color;
    float intensity;
};

layout(std140, binding = 3) uniform LightData {
    DirectionalLight u_DirectionalLight;
    int u_PointLightCount;
    // PointLight u_PointLights[MAX_POINT_LIGHTS];
};
*/
```

---

## 4. UniformBuffer 与 GPU 数据管理

### 4.1 为什么需要 UniformBuffer？

**当前问题**：
```cpp
// 每帧每个物体都要调用多次 glGetUniformLocation + glUniform*
shader->SetMat4("u_ViewProjection", camera.GetViewProjection());
shader->SetMat4("u_Model", transform);
shader->SetVec3("u_CameraPosition", camera.GetPosition());
// ... 更多 uniform
```

**问题**：
1. `glGetUniformLocation` 开销大（即使缓存了）
2. 每个 `glUniform*` 都是一次 CPU→GPU 传输
3. 无法批量更新

**解决方案**：使用 Uniform Buffer Object (UBO)
```cpp
// 一次性上传所有相机数据
struct CameraData {
    glm::mat4 viewProjection;
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec3 position;
    float padding;  // 对齐
};

cameraUBO->SetData(&cameraData, sizeof(CameraData));
```

### 4.2 UniformBuffer 设计

```cpp
// ============ 抽象接口 ============

class UniformBuffer : public RefCounter {
public:
    virtual ~UniformBuffer() = default;

    // 创建时指定大小和绑定点
    static Ref<UniformBuffer> Create(uint32_t size, uint32_t bindingPoint);

    // 更新数据
    virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) = 0;

    // 绑定到管线
    virtual void Bind() const = 0;
    virtual void BindToSlot(uint32_t slot) const = 0;

    // 获取信息
    uint32_t GetSize() const { return m_size; }
    uint32_t GetBindingPoint() const { return m_bindingPoint; }

protected:
    uint32_t m_size = 0;
    uint32_t m_bindingPoint = 0;
};

// ============ OpenGL 实现 ============

class OpenGLUniformBuffer : public UniformBuffer {
public:
    OpenGLUniformBuffer(uint32_t size, uint32_t bindingPoint) {
        m_size = size;
        m_bindingPoint = bindingPoint;

        glCreateBuffers(1, &m_rendererId);
        glNamedBufferData(m_rendererId, size, nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, m_rendererId);
    }

    ~OpenGLUniformBuffer() {
        glDeleteBuffers(1, &m_rendererId);
    }

    void SetData(const void* data, uint32_t size, uint32_t offset = 0) override {
        glNamedBufferSubData(m_rendererId, offset, size, data);
    }

    void Bind() const override {
        glBindBufferBase(GL_UNIFORM_BUFFER, m_bindingPoint, m_rendererId);
    }

    void BindToSlot(uint32_t slot) const override {
        glBindBufferBase(GL_UNIFORM_BUFFER, slot, m_rendererId);
    }

private:
    uint32_t m_rendererId = 0;
};
```

### 4.3 标准化的 Binding Point 约定

```cpp
// 定义标准绑定点，所有 Shader 遵循
namespace UniformBindingPoints {
    constexpr uint32_t Camera = 0;
    constexpr uint32_t Transform = 1;
    constexpr uint32_t Material = 2;
    constexpr uint32_t Lighting = 3;
    constexpr uint32_t Environment = 4;
    constexpr uint32_t Skinning = 5;    // 骨骼动画
    constexpr uint32_t Custom = 10;     // 用户自定义起始点
}

// GPU 端结构体定义 (std140 布局)
// 需要注意内存对齐！

// Camera UBO - Binding 0
struct alignas(16) CameraData {
    glm::mat4 viewProjection;  // offset 0,   size 64
    glm::mat4 view;            // offset 64,  size 64
    glm::mat4 projection;      // offset 128, size 64
    glm::vec3 position;        // offset 192, size 12
    float nearPlane;           // offset 204, size 4
    float farPlane;            // offset 208, size 4
    float _padding[3];         // offset 212, size 12 (对齐到 224)
};

// Transform UBO - Binding 1 (Per-Object)
struct alignas(16) TransformData {
    glm::mat4 model;           // offset 0,  size 64
    glm::mat4 normalMatrix;    // offset 64, size 64 (mat3 需要特殊处理)
};

// Material UBO - Binding 2
struct alignas(16) MaterialData {
    glm::vec4 albedo;          // offset 0,  size 16
    float metallic;            // offset 16, size 4
    float roughness;           // offset 20, size 4
    float ao;                  // offset 24, size 4
    float _padding;            // offset 28, size 4
    glm::vec4 emissive;        // offset 32, size 16
};

// Light UBO - Binding 3
struct alignas(16) DirectionalLightData {
    glm::vec4 direction;       // vec3 + padding
    glm::vec4 color;           // vec3 + intensity
};

struct alignas(16) PointLightData {
    glm::vec4 position;        // vec3 + radius
    glm::vec4 color;           // vec3 + intensity
};

struct alignas(16) LightingData {
    DirectionalLightData directionalLight;
    glm::vec4 ambientColor;    // vec3 + intensity
    int pointLightCount;
    int _padding[3];
    PointLightData pointLights[16];  // 最大16个点光源
};
```

### 4.4 std140 布局规则

```cpp
/*
std140 布局规则 (GLSL 标准):

1. 标量 (float, int, bool):
   - Size: 4 bytes
   - Alignment: 4 bytes

2. vec2:
   - Size: 8 bytes
   - Alignment: 8 bytes

3. vec3:
   - Size: 12 bytes
   - Alignment: 16 bytes (注意！)

4. vec4:
   - Size: 16 bytes
   - Alignment: 16 bytes

5. mat4:
   - Size: 64 bytes (4 * vec4)
   - Alignment: 16 bytes

6. 数组:
   - 每个元素对齐到 16 bytes
   - float[4] 实际占用 64 bytes, 不是 16 bytes!

7. 结构体:
   - 对齐到其最大成员的对齐值，向上取整到 16 的倍数
*/

// 错误示例
struct BadLayout {
    glm::vec3 position;  // 12 bytes
    float radius;        // 4 bytes - 这里 radius 会被放在下一个 16 字节块！
};
// 实际布局: position(16) + radius(4) + padding(12) = 32 bytes

// 正确做法
struct GoodLayout {
    glm::vec3 position;
    float radius;        // 紧跟在 vec3 后面，共用 16 字节
};
// 实际布局: 16 bytes

// 或者使用 vec4
struct BetterLayout {
    glm::vec4 positionAndRadius;  // xyz = position, w = radius
};
```

### 4.5 UniformBuffer 管理器

```cpp
class UniformBufferManager {
public:
    void Init() {
        // 创建标准 UBO
        m_cameraBuffer = UniformBuffer::Create(sizeof(CameraData),
                                                UniformBindingPoints::Camera);
        m_transformBuffer = UniformBuffer::Create(sizeof(TransformData),
                                                   UniformBindingPoints::Transform);
        m_materialBuffer = UniformBuffer::Create(sizeof(MaterialData),
                                                  UniformBindingPoints::Material);
        m_lightingBuffer = UniformBuffer::Create(sizeof(LightingData),
                                                  UniformBindingPoints::Lighting);
    }

    void UpdateCamera(const Camera& camera) {
        CameraData data;
        data.viewProjection = camera.GetViewProjection();
        data.view = camera.GetViewMatrix();
        data.projection = camera.GetProjectionMatrix();
        data.position = camera.GetPosition();
        data.nearPlane = camera.GetNearPlane();
        data.farPlane = camera.GetFarPlane();

        m_cameraBuffer->SetData(&data, sizeof(data));
    }

    void UpdateTransform(const glm::mat4& model) {
        TransformData data;
        data.model = model;
        data.normalMatrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(model))));

        m_transformBuffer->SetData(&data, sizeof(data));
    }

    void UpdateLighting(const LightEnvironment& env) {
        LightingData data = {};
        // 填充光照数据...
        m_lightingBuffer->SetData(&data, sizeof(data));
    }

private:
    Ref<UniformBuffer> m_cameraBuffer;
    Ref<UniformBuffer> m_transformBuffer;
    Ref<UniformBuffer> m_materialBuffer;
    Ref<UniformBuffer> m_lightingBuffer;
};
```

---

## 5. Material 系统设计

### 5.1 Material 的核心职责

Material 是连接 Shader 和渲染数据的桥梁：

```
Shader (程序)
    ↓
Material (数据实例)
    ↓
MeshRenderer (使用者)
```

### 5.2 设计模式分析

#### 选项 A：数据驱动（推荐）

```cpp
// Material 持有 Shader，存储 Uniform 数据
class Material : public RefCounter {
private:
    Ref<Shader> m_shader;
    Buffer m_uniformStorage;  // CPU 端数据
    std::unordered_map<std::string, Ref<Texture>> m_textures;
    uint32_t m_flags;
};
```

**优点**：
- 灵活性高，可动态改变属性
- 易于序列化
- 支持材质实例化

**缺点**：
- 每次绑定需要遍历属性
- 类型不安全（运行时查找）

#### 选项 B：模板化材质（特定用途）

```cpp
// 编译时类型安全，但不够灵活
template<typename MaterialDataT>
class TypedMaterial {
    Ref<Shader> m_shader;
    MaterialDataT m_data;  // 结构体
};

// 使用
struct PBRMaterialData {
    glm::vec4 albedo;
    float metallic;
    float roughness;
    Ref<Texture2D> albedoMap;
    Ref<Texture2D> normalMap;
};

using PBRMaterial = TypedMaterial<PBRMaterialData>;
```

### 5.3 完整的 Material 实现

```cpp
// ============ Material Flags ============

enum class MaterialFlag : uint32_t {
    None           = 0,
    DepthTest      = BIT(0),
    DepthWrite     = BIT(1),
    Blend          = BIT(2),
    TwoSided       = BIT(3),
    Wireframe      = BIT(4),
    DisableShadow  = BIT(5),
    Transparent    = BIT(6),

    // 常用组合
    Default = DepthTest | DepthWrite,
    Translucent = DepthTest | Blend | Transparent,
};

MINT_ENUM_FLAGS(MaterialFlag);

// ============ Blend Mode ============

enum class BlendMode {
    Opaque,
    Alpha,
    Additive,
    Multiply
};

// ============ Material 核心类 ============

class Material : public RefCounter, public Asset {
public:
    Material() = default;
    explicit Material(const Ref<Shader>& shader);

    // ============ 属性设置 ============

    template<typename T>
    void Set(const std::string& name, const T& value) {
        auto* uniform = FindUniform(name);
        if (!uniform) {
            MINT_WARN("Material: Uniform '{}' not found", name);
            return;
        }

        // 类型检查
        if (GetShaderUniformType<T>() != uniform->type) {
            MINT_ERROR("Material: Type mismatch for uniform '{}'", name);
            return;
        }

        // 写入存储
        m_uniformStorage.Write(&value, sizeof(T), uniform->offset);
        m_dirty = true;
    }

    void Set(const std::string& name, const Ref<Texture2D>& texture) {
        m_textures[name] = texture;
    }

    void Set(const std::string& name, const Ref<TextureCube>& texture) {
        m_textureCubes[name] = texture;
    }

    // ============ 属性获取 ============

    template<typename T>
    T& Get(const std::string& name) {
        auto* uniform = FindUniform(name);
        MINT_ASSERT(uniform, "Uniform not found");
        return m_uniformStorage.Read<T>(uniform->offset);
    }

    template<typename T>
    const T& Get(const std::string& name) const {
        return const_cast<Material*>(this)->Get<T>(name);
    }

    Ref<Texture2D> GetTexture(const std::string& name) const {
        auto it = m_textures.find(name);
        return it != m_textures.end() ? it->second : nullptr;
    }

    // ============ 渲染状态 ============

    void SetFlag(MaterialFlag flag, bool value = true) {
        if (value) m_flags |= (uint32_t)flag;
        else m_flags &= ~(uint32_t)flag;
    }

    bool HasFlag(MaterialFlag flag) const {
        return (m_flags & (uint32_t)flag) != 0;
    }

    void SetBlendMode(BlendMode mode) { m_blendMode = mode; }
    BlendMode GetBlendMode() const { return m_blendMode; }

    // ============ 绑定到渲染管线 ============

    void Bind() {
        m_shader->Bind();

        // 上传 Uniforms
        if (m_dirty) {
            UploadUniforms();
            m_dirty = false;
        }

        // 绑定纹理
        uint32_t textureSlot = 0;
        for (const auto& [name, texture] : m_textures) {
            if (texture) {
                texture->Bind(textureSlot);
                m_shader->SetInt(name, textureSlot);
                textureSlot++;
            }
        }

        for (const auto& [name, texture] : m_textureCubes) {
            if (texture) {
                texture->Bind(textureSlot);
                m_shader->SetInt(name, textureSlot);
                textureSlot++;
            }
        }

        // 应用渲染状态
        ApplyRenderState();
    }

    // ============ 工厂方法 ============

    static Ref<Material> Create(const Ref<Shader>& shader);

    // 从 Shader 反射数据自动创建 Material，初始化默认值
    static Ref<Material> CreateFromShader(const Ref<Shader>& shader);

    // ============ Getter ============

    Ref<Shader> GetShader() const { return m_shader; }

    // Asset 接口
    AssetType GetAssetType() const override { return AssetType::Material; }

private:
    void UploadUniforms();
    void ApplyRenderState();
    const ShaderUniform* FindUniform(const std::string& name) const;

private:
    Ref<Shader> m_shader;

    // Uniform 存储
    Buffer m_uniformStorage;
    std::unordered_map<std::string, uint32_t> m_uniformOffsets;

    // 纹理
    std::unordered_map<std::string, Ref<Texture2D>> m_textures;
    std::unordered_map<std::string, Ref<TextureCube>> m_textureCubes;

    // 渲染状态
    uint32_t m_flags = (uint32_t)MaterialFlag::Default;
    BlendMode m_blendMode = BlendMode::Opaque;

    // 脏标记
    bool m_dirty = true;
};
```

### 5.4 MaterialInstance - 材质变体

```cpp
// MaterialInstance 共享 Shader 和基础属性，但可覆盖特定值
class MaterialInstance : public RefCounter {
public:
    explicit MaterialInstance(const Ref<Material>& parent)
        : m_parent(parent) {
        // 复制父材质的 Uniform 存储
        m_overrideStorage = parent->GetUniformStorage().Clone();
    }

    template<typename T>
    void Set(const std::string& name, const T& value) {
        m_overriddenProperties.insert(name);
        // 写入覆盖存储
        auto* uniform = m_parent->FindUniform(name);
        if (uniform) {
            m_overrideStorage.Write(&value, sizeof(T), uniform->offset);
        }
    }

    void Bind() {
        // 使用父材质的 Shader
        m_parent->GetShader()->Bind();

        // 上传覆盖后的 Uniforms
        UploadUniforms();

        // 绑定纹理（优先使用覆盖的）
        BindTextures();
    }

private:
    Ref<Material> m_parent;
    Buffer m_overrideStorage;
    std::set<std::string> m_overriddenProperties;
    std::unordered_map<std::string, Ref<Texture2D>> m_overrideTextures;
};

// 使用示例
auto baseMaterial = Material::Create(pbrShader);
baseMaterial->Set("u_Albedo", glm::vec4(1.0f));
baseMaterial->Set("u_Metallic", 0.5f);

auto instance1 = Ref<MaterialInstance>::Create(baseMaterial);
instance1->Set("u_Albedo", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));  // 红色变体

auto instance2 = Ref<MaterialInstance>::Create(baseMaterial);
instance2->Set("u_Albedo", glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));  // 绿色变体
```

### 5.5 预定义材质类型

```cpp
// 常用材质的工厂类
class MaterialFactory {
public:
    static Ref<Material> CreateUnlit(const glm::vec4& color) {
        auto shader = ShaderLibrary::Get("Unlit");
        auto material = Material::Create(shader);
        material->Set("u_Color", color);
        return material;
    }

    static Ref<Material> CreatePBR(
        const glm::vec4& albedo = glm::vec4(1.0f),
        float metallic = 0.0f,
        float roughness = 0.5f
    ) {
        auto shader = ShaderLibrary::Get("PBR");
        auto material = Material::Create(shader);
        material->Set("u_Albedo", albedo);
        material->Set("u_Metallic", metallic);
        material->Set("u_Roughness", roughness);
        return material;
    }

    static Ref<Material> CreateSkybox(const Ref<TextureCube>& cubemap) {
        auto shader = ShaderLibrary::Get("Skybox");
        auto material = Material::Create(shader);
        material->Set("u_Skybox", cubemap);
        material->SetFlag(MaterialFlag::DepthWrite, false);
        return material;
    }
};
```

---

## 6. Mesh 系统架构

### 6.1 为什么需要 Resource/Instance 分离？

**问题场景**：
```cpp
// 假设场景中有100棵相同的树
for (int i = 0; i < 100; i++) {
    auto mesh = Mesh::Create("tree.fbx");  // 每次都加载?
    mesh->SetMaterial(treeMaterial);
    mesh->SetTransform(positions[i]);
}
```

**问题**：
1. 相同的顶点数据被加载100次
2. 无法区分"模型数据"和"使用方式"

**解决方案**：Resource/Instance 分离
```cpp
// Resource: 共享的模型数据
auto treeResource = MeshResource::Load("tree.fbx");

// Instance: 每个树的独立实例
for (int i = 0; i < 100; i++) {
    auto tree = StaticMesh::Create(treeResource);
    tree->SetMaterial(0, greenMaterial);  // 可以有不同材质
    tree->SetTransform(positions[i]);
}
```

### 6.2 Mesh 系统架构

```cpp
// ============ SubMesh - 渲染原子单元 ============

struct SubMesh {
    uint32_t baseVertex;
    uint32_t baseIndex;
    uint32_t indexCount;
    uint32_t materialIndex;

    AABB boundingBox;
    glm::mat4 localTransform = glm::mat4(1.0f);

    std::string name;  // 调试用
};

// ============ MeshResource - 共享的GPU数据 ============

class MeshResource : public Asset {
public:
    static Ref<MeshResource> Create(const std::vector<Vertex>& vertices,
                                     const std::vector<uint32_t>& indices);
    static Ref<MeshResource> Load(const std::filesystem::path& filepath);

    // 渲染
    void Bind() const { m_vertexArray->Bind(); }

    void Draw() const {
        m_vertexArray->Bind();
        glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, nullptr);
    }

    void DrawSubMesh(const SubMesh& submesh) const {
        m_vertexArray->Bind();
        glDrawElementsBaseVertex(
            GL_TRIANGLES,
            submesh.indexCount,
            GL_UNSIGNED_INT,
            (void*)(submesh.baseIndex * sizeof(uint32_t)),
            submesh.baseVertex
        );
    }

    // 数据访问
    const std::vector<SubMesh>& GetSubMeshes() const { return m_submeshes; }
    const AABB& GetBoundingBox() const { return m_boundingBox; }
    uint32_t GetMaterialSlotCount() const { return m_materialSlotCount; }

    // Asset 接口
    AssetType GetAssetType() const override { return AssetType::Mesh; }

private:
    Ref<VertexArray> m_vertexArray;
    uint32_t m_vertexCount = 0;
    uint32_t m_indexCount = 0;

    std::vector<SubMesh> m_submeshes;
    AABB m_boundingBox;

    uint32_t m_materialSlotCount = 0;
    std::vector<std::string> m_materialSlotNames;

    // 原始数据（可选保留，用于CPU端碰撞检测等）
    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;
};

// ============ MaterialTable - 材质槽管理 ============

class MaterialTable : public RefCounter {
public:
    explicit MaterialTable(uint32_t slotCount)
        : m_materials(slotCount) {}

    void SetMaterial(uint32_t slot, const Ref<Material>& material) {
        if (slot < m_materials.size()) {
            m_materials[slot] = material;
        }
    }

    Ref<Material> GetMaterial(uint32_t slot) const {
        return slot < m_materials.size() ? m_materials[slot] : nullptr;
    }

    bool HasMaterial(uint32_t slot) const {
        return slot < m_materials.size() && m_materials[slot] != nullptr;
    }

    uint32_t GetSlotCount() const { return (uint32_t)m_materials.size(); }

private:
    std::vector<Ref<Material>> m_materials;
};

// ============ StaticMesh - 场景中的实例 ============

class StaticMesh : public RefCounter {
public:
    explicit StaticMesh(const Ref<MeshResource>& resource)
        : m_resource(resource)
        , m_materialTable(Ref<MaterialTable>::Create(resource->GetMaterialSlotCount()))
    {}

    // 材质管理
    void SetMaterial(uint32_t slot, const Ref<Material>& material) {
        m_materialTable->SetMaterial(slot, material);
    }

    Ref<Material> GetMaterial(uint32_t slot) const {
        return m_materialTable->GetMaterial(slot);
    }

    // SubMesh 可见性控制
    void SetSubMeshVisible(uint32_t index, bool visible) {
        if (index < m_submeshVisibility.size()) {
            m_submeshVisibility[index] = visible;
        }
    }

    bool IsSubMeshVisible(uint32_t index) const {
        return index < m_submeshVisibility.size() ? m_submeshVisibility[index] : true;
    }

    // 渲染
    void Render(const glm::mat4& transform, const Ref<Material>& defaultMaterial = nullptr) {
        m_resource->Bind();

        for (size_t i = 0; i < m_resource->GetSubMeshes().size(); i++) {
            if (!IsSubMeshVisible(i)) continue;

            const auto& submesh = m_resource->GetSubMeshes()[i];

            // 获取材质
            auto material = m_materialTable->GetMaterial(submesh.materialIndex);
            if (!material) material = defaultMaterial;
            if (!material) continue;

            // 绑定材质并设置变换
            material->Bind();
            material->GetShader()->SetMat4("u_Model", transform * submesh.localTransform);

            // 绘制
            m_resource->DrawSubMesh(submesh);
        }
    }

    // Getter
    Ref<MeshResource> GetResource() const { return m_resource; }
    Ref<MaterialTable> GetMaterialTable() const { return m_materialTable; }

private:
    Ref<MeshResource> m_resource;
    Ref<MaterialTable> m_materialTable;
    std::vector<bool> m_submeshVisibility;
};
```

### 6.3 模型加载（使用 Assimp）

```cpp
class MeshLoader {
public:
    static Ref<MeshResource> Load(const std::filesystem::path& filepath) {
        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFile(
            filepath.string(),
            aiProcess_Triangulate |
            aiProcess_GenSmoothNormals |
            aiProcess_CalcTangentSpace |
            aiProcess_JoinIdenticalVertices |
            aiProcess_OptimizeMeshes |
            aiProcess_ValidateDataStructure
        );

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
            MINT_ERROR("Assimp: {}", importer.GetErrorString());
            return nullptr;
        }

        MeshLoader loader;
        loader.ProcessNode(scene->mRootNode, scene, glm::mat4(1.0f));

        return loader.CreateMeshResource();
    }

private:
    void ProcessNode(aiNode* node, const aiScene* scene, const glm::mat4& parentTransform) {
        glm::mat4 nodeTransform = parentTransform * AssimpToGLM(node->mTransformation);

        for (uint32_t i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            ProcessMesh(mesh, scene, nodeTransform);
        }

        for (uint32_t i = 0; i < node->mNumChildren; i++) {
            ProcessNode(node->mChildren[i], scene, nodeTransform);
        }
    }

    void ProcessMesh(aiMesh* mesh, const aiScene* scene, const glm::mat4& transform) {
        uint32_t baseVertex = (uint32_t)m_vertices.size();
        uint32_t baseIndex = (uint32_t)m_indices.size();

        // 顶点数据
        for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
            Vertex vertex;

            vertex.position = {
                mesh->mVertices[i].x,
                mesh->mVertices[i].y,
                mesh->mVertices[i].z
            };

            if (mesh->HasNormals()) {
                vertex.normal = {
                    mesh->mNormals[i].x,
                    mesh->mNormals[i].y,
                    mesh->mNormals[i].z
                };
            }

            if (mesh->HasTangentsAndBitangents()) {
                vertex.tangent = {
                    mesh->mTangents[i].x,
                    mesh->mTangents[i].y,
                    mesh->mTangents[i].z
                };
                vertex.bitangent = {
                    mesh->mBitangents[i].x,
                    mesh->mBitangents[i].y,
                    mesh->mBitangents[i].z
                };
            }

            if (mesh->HasTextureCoords(0)) {
                vertex.texCoord = {
                    mesh->mTextureCoords[0][i].x,
                    mesh->mTextureCoords[0][i].y
                };
            }

            m_vertices.push_back(vertex);
        }

        // 索引数据
        for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
            aiFace& face = mesh->mFaces[i];
            for (uint32_t j = 0; j < face.mNumIndices; j++) {
                m_indices.push_back(face.mIndices[j]);
            }
        }

        // 创建 SubMesh
        SubMesh submesh;
        submesh.baseVertex = baseVertex;
        submesh.baseIndex = baseIndex;
        submesh.indexCount = (uint32_t)m_indices.size() - baseIndex;
        submesh.materialIndex = mesh->mMaterialIndex;
        submesh.localTransform = transform;
        submesh.name = mesh->mName.C_Str();

        // 计算包围盒
        submesh.boundingBox = CalculateAABB(m_vertices, baseVertex,
                                            (uint32_t)m_vertices.size() - baseVertex);

        m_submeshes.push_back(submesh);
        m_materialSlotCount = std::max(m_materialSlotCount, mesh->mMaterialIndex + 1);
    }

    Ref<MeshResource> CreateMeshResource() {
        auto resource = Ref<MeshResource>::Create();
        resource->m_vertices = std::move(m_vertices);
        resource->m_indices = std::move(m_indices);
        resource->m_submeshes = std::move(m_submeshes);
        resource->m_materialSlotCount = m_materialSlotCount;
        resource->CreateGPUBuffers();
        return resource;
    }

private:
    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;
    std::vector<SubMesh> m_submeshes;
    uint32_t m_materialSlotCount = 0;
};
```

---

## 7. 资源管理系统

### 7.1 资源系统设计目标

1. **唯一性**：相同路径的资源只加载一次
2. **生命周期**：自动管理加载/卸载
3. **类型安全**：编译时类型检查
4. **异步加载**：不阻塞主线程（可选）

### 7.2 Asset 基类设计

```cpp
// ============ Asset Handle ============

// 使用 UUID 作为唯一标识
struct AssetHandle {
    uint64_t value = 0;

    bool IsValid() const { return value != 0; }

    bool operator==(const AssetHandle& other) const { return value == other.value; }
    bool operator!=(const AssetHandle& other) const { return value != other.value; }

    static AssetHandle Generate() { return { UUID::Generate() }; }
};

// Hash 支持
template<>
struct std::hash<AssetHandle> {
    size_t operator()(const AssetHandle& handle) const {
        return std::hash<uint64_t>{}(handle.value);
    }
};

// ============ Asset 元数据 ============

enum class AssetType : uint16_t {
    None = 0,
    Texture2D,
    TextureCube,
    Shader,
    Material,
    Mesh,
    Scene,
    Audio,
    Font,
    Prefab,
};

enum class AssetState : uint8_t {
    Unloaded,
    Loading,
    Ready,
    Failed,
};

struct AssetMetadata {
    AssetHandle handle;
    AssetType type = AssetType::None;
    std::filesystem::path filePath;
    std::filesystem::path sourcePath;  // 原始资源路径（用于热重载）
    bool isMemoryOnly = false;

    // 可选：版本/依赖信息
    uint32_t version = 0;
    std::vector<AssetHandle> dependencies;
};

// ============ Asset 基类 ============

class Asset : public RefCounter {
public:
    virtual ~Asset() = default;

    virtual AssetType GetAssetType() const = 0;
    virtual bool IsValid() const { return true; }

    AssetHandle GetHandle() const { return m_handle; }
    const std::filesystem::path& GetPath() const { return m_path; }
    AssetState GetState() const { return m_state; }

protected:
    friend class AssetManager;

    AssetHandle m_handle;
    std::filesystem::path m_path;
    AssetState m_state = AssetState::Unloaded;
};

// ============ 类型特化宏 ============

#define MINT_ASSET_TYPE(type) \
    static AssetType GetStaticType() { return AssetType::type; } \
    AssetType GetAssetType() const override { return GetStaticType(); }
```

### 7.3 AssetManager 设计

```cpp
class AssetManager {
public:
    // ============ 单例访问 ============
    static AssetManager& Get() {
        static AssetManager instance;
        return instance;
    }

    // ============ 资源加载 ============

    template<typename T>
    Ref<T> LoadAsset(const std::filesystem::path& path) {
        static_assert(std::is_base_of_v<Asset, T>, "T must derive from Asset");

        // 检查是否已加载
        auto it = m_pathToHandle.find(path);
        if (it != m_pathToHandle.end()) {
            return GetAsset<T>(it->second);
        }

        // 创建句柄和元数据
        AssetHandle handle = AssetHandle::Generate();
        AssetMetadata metadata;
        metadata.handle = handle;
        metadata.type = T::GetStaticType();
        metadata.filePath = path;

        // 调用类型特定的加载器
        Ref<T> asset = LoadAssetInternal<T>(path);
        if (!asset) {
            MINT_ERROR("Failed to load asset: {}", path.string());
            return nullptr;
        }

        // 设置 Asset 元数据
        asset->m_handle = handle;
        asset->m_path = path;
        asset->m_state = AssetState::Ready;

        // 注册
        m_registry[handle] = metadata;
        m_loadedAssets[handle] = asset;
        m_pathToHandle[path] = handle;

        return asset;
    }

    template<typename T>
    Ref<T> GetAsset(AssetHandle handle) {
        auto it = m_loadedAssets.find(handle);
        if (it != m_loadedAssets.end()) {
            return std::static_pointer_cast<T>(it->second);
        }
        return nullptr;
    }

    // ============ 资源管理 ============

    bool IsAssetLoaded(AssetHandle handle) const {
        return m_loadedAssets.find(handle) != m_loadedAssets.end();
    }

    const AssetMetadata* GetMetadata(AssetHandle handle) const {
        auto it = m_registry.find(handle);
        return it != m_registry.end() ? &it->second : nullptr;
    }

    void UnloadAsset(AssetHandle handle) {
        auto it = m_loadedAssets.find(handle);
        if (it != m_loadedAssets.end()) {
            // 从路径映射中移除
            const auto& metadata = m_registry[handle];
            m_pathToHandle.erase(metadata.filePath);

            // 移除资源
            m_loadedAssets.erase(it);
        }
    }

    // 卸载引用计数为1的资源（只有 AssetManager 持有）
    void UnloadUnusedAssets() {
        std::vector<AssetHandle> toUnload;

        for (auto& [handle, asset] : m_loadedAssets) {
            if (asset.use_count() == 1) {
                toUnload.push_back(handle);
            }
        }

        for (auto handle : toUnload) {
            UnloadAsset(handle);
        }
    }

    // ============ 热重载支持 ============

    void ReloadAsset(AssetHandle handle) {
        auto* metadata = GetMetadata(handle);
        if (!metadata) return;

        // 标记为加载中
        auto& asset = m_loadedAssets[handle];
        asset->m_state = AssetState::Loading;

        // 重新加载
        // 这里需要类型分发...
    }

private:
    // 类型特定的加载器
    template<typename T>
    Ref<T> LoadAssetInternal(const std::filesystem::path& path);

private:
    std::unordered_map<AssetHandle, AssetMetadata> m_registry;
    std::unordered_map<AssetHandle, Ref<Asset>> m_loadedAssets;
    std::unordered_map<std::filesystem::path, AssetHandle> m_pathToHandle;
};

// ============ 加载器特化 ============

template<>
Ref<Texture2D> AssetManager::LoadAssetInternal<Texture2D>(const std::filesystem::path& path) {
    return Texture2D::Create({}, path);
}

template<>
Ref<Shader> AssetManager::LoadAssetInternal<Shader>(const std::filesystem::path& path) {
    return Shader::Create(path);
}

template<>
Ref<MeshResource> AssetManager::LoadAssetInternal<MeshResource>(const std::filesystem::path& path) {
    return MeshLoader::Load(path);
}
```

### 7.4 资源的使用方式

```cpp
// 方式1: 直接路径加载（推荐日常使用）
auto texture = AssetManager::Get().LoadAsset<Texture2D>("textures/wood.png");
auto mesh = AssetManager::Get().LoadAsset<MeshResource>("models/character.fbx");

// 方式2: 通过 Handle 获取（用于序列化后恢复）
AssetHandle savedHandle = ...; // 从文件加载
auto material = AssetManager::Get().GetAsset<Material>(savedHandle);

// 方式3: 检查加载状态
if (texture->GetState() == AssetState::Ready) {
    // 可以使用
}
```

---

## 8. 场景与 ECS 系统

### 8.1 为什么选择 ECS？

**传统继承方式的问题**：

```cpp
// 深度继承树
class Entity { };
class Actor : public Entity { };
class Pawn : public Actor { };
class Character : public Pawn { };
class Enemy : public Character { };
class FlyingEnemy : public Enemy { };  // 如果敌人会飞？
class SwimmingEnemy : public Enemy { };  // 如果敌人会游泳？
class FlyingSwimmingEnemy : public ??? { };  // 问题！
```

**ECS 的优势**：组合优于继承

```cpp
// 组件组合
auto enemy = scene.CreateEntity("Enemy");
enemy.AddComponent<TransformComponent>();
enemy.AddComponent<MeshRendererComponent>();
enemy.AddComponent<HealthComponent>();
enemy.AddComponent<AIComponent>();

// 可飞行
if (canFly) enemy.AddComponent<FlyingComponent>();

// 可游泳
if (canSwim) enemy.AddComponent<SwimmingComponent>();

// 同时会飞会游泳？没问题！
```

### 8.2 使用 entt 的 ECS 实现

```cpp
// ============ 前向声明 ============

class Scene;
class Entity;

// ============ Scene ============

class Scene : public RefCounter, public Asset {
public:
    Scene() = default;
    ~Scene();

    // ============ Entity 管理 ============

    Entity CreateEntity(const std::string& name = "Entity");
    Entity CreateEntityWithUUID(UUID uuid, const std::string& name = "Entity");
    void DestroyEntity(Entity entity);

    Entity FindEntityByName(const std::string& name);
    Entity FindEntityByUUID(UUID uuid);

    // ============ 更新 ============

    void OnUpdate(TimeStep ts);
    void OnRender(EditorCamera& camera);
    void OnRender(const Camera& camera, const glm::mat4& transform);

    // ============ 组件查询 ============

    template<typename... Components>
    auto GetAllEntitiesWith() {
        return m_registry.view<Components...>();
    }

    // ============ 生命周期 ============

    void OnRuntimeStart();
    void OnRuntimeStop();

    // Asset 接口
    MINT_ASSET_TYPE(Scene);

private:
    friend class Entity;
    friend class SceneSerializer;

    entt::registry m_registry;
    std::unordered_map<UUID, entt::entity> m_entityMap;

    // 场景状态
    bool m_isRunning = false;
};

// ============ Entity ============

class Entity {
public:
    Entity() = default;
    Entity(entt::entity handle, Scene* scene)
        : m_entityHandle(handle), m_scene(scene) {}

    // ============ 组件操作 ============

    template<typename T, typename... Args>
    T& AddComponent(Args&&... args) {
        MINT_ASSERT(!HasComponent<T>(), "Entity already has component");
        return m_scene->m_registry.emplace<T>(m_entityHandle, std::forward<Args>(args)...);
    }

    template<typename T, typename... Args>
    T& AddOrReplaceComponent(Args&&... args) {
        return m_scene->m_registry.emplace_or_replace<T>(m_entityHandle, std::forward<Args>(args)...);
    }

    template<typename T>
    T& GetComponent() {
        MINT_ASSERT(HasComponent<T>(), "Entity does not have component");
        return m_scene->m_registry.get<T>(m_entityHandle);
    }

    template<typename T>
    const T& GetComponent() const {
        return const_cast<Entity*>(this)->GetComponent<T>();
    }

    template<typename T>
    bool HasComponent() const {
        return m_scene->m_registry.all_of<T>(m_entityHandle);
    }

    template<typename T>
    void RemoveComponent() {
        MINT_ASSERT(HasComponent<T>(), "Entity does not have component");
        m_scene->m_registry.remove<T>(m_entityHandle);
    }

    // ============ 辅助方法 ============

    UUID GetUUID() { return GetComponent<IDComponent>().id; }
    const std::string& GetName() { return GetComponent<TagComponent>().tag; }

    TransformComponent& GetTransform() { return GetComponent<TransformComponent>(); }

    // ============ 运算符 ============

    operator bool() const { return m_entityHandle != entt::null && m_scene != nullptr; }
    operator entt::entity() const { return m_entityHandle; }
    operator uint32_t() const { return (uint32_t)m_entityHandle; }

    bool operator==(const Entity& other) const {
        return m_entityHandle == other.m_entityHandle && m_scene == other.m_scene;
    }

    bool operator!=(const Entity& other) const { return !(*this == other); }

private:
    entt::entity m_entityHandle = entt::null;
    Scene* m_scene = nullptr;
};
```

### 8.3 组件定义

```cpp
// ============ 核心组件 ============

struct IDComponent {
    UUID id;

    IDComponent() = default;
    IDComponent(UUID uuid) : id(uuid) {}
};

struct TagComponent {
    std::string tag;

    TagComponent() = default;
    TagComponent(const std::string& t) : tag(t) {}
};

struct TransformComponent {
    glm::vec3 translation = { 0.0f, 0.0f, 0.0f };
    glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };  // Euler angles (degrees)
    glm::vec3 scale = { 1.0f, 1.0f, 1.0f };

    TransformComponent() = default;

    glm::mat4 GetTransform() const {
        return glm::translate(glm::mat4(1.0f), translation)
             * glm::toMat4(glm::quat(glm::radians(rotation)))
             * glm::scale(glm::mat4(1.0f), scale);
    }
};

// ============ 渲染组件 ============

struct MeshRendererComponent {
    Ref<StaticMesh> mesh;
    bool visible = true;
    bool castShadow = true;
    bool receiveShadow = true;

    MeshRendererComponent() = default;
    MeshRendererComponent(const Ref<StaticMesh>& m) : mesh(m) {}
};

struct CameraComponent {
    SceneCamera camera;
    bool primary = true;
    bool fixedAspectRatio = false;

    CameraComponent() = default;
};

// ============ 光照组件 ============

struct DirectionalLightComponent {
    glm::vec3 color = { 1.0f, 1.0f, 1.0f };
    float intensity = 1.0f;
    bool castShadow = true;

    DirectionalLightComponent() = default;
};

struct PointLightComponent {
    glm::vec3 color = { 1.0f, 1.0f, 1.0f };
    float intensity = 1.0f;
    float radius = 10.0f;
    float falloff = 1.0f;

    PointLightComponent() = default;
};

struct SpotLightComponent {
    glm::vec3 color = { 1.0f, 1.0f, 1.0f };
    float intensity = 1.0f;
    float innerConeAngle = 30.0f;  // degrees
    float outerConeAngle = 45.0f;
    float range = 10.0f;

    SpotLightComponent() = default;
};

// ============ 物理组件（未来） ============

struct RigidbodyComponent {
    enum class BodyType { Static, Kinematic, Dynamic };
    BodyType type = BodyType::Static;
    float mass = 1.0f;
    float linearDamping = 0.0f;
    float angularDamping = 0.05f;
    bool useGravity = true;

    // 运行时物理引擎数据
    void* runtimeBody = nullptr;
};

struct BoxColliderComponent {
    glm::vec3 size = { 1.0f, 1.0f, 1.0f };
    glm::vec3 offset = { 0.0f, 0.0f, 0.0f };
    bool isTrigger = false;

    // 物理材质
    float friction = 0.5f;
    float restitution = 0.0f;
};
```

### 8.4 Scene 渲染实现

```cpp
void Scene::OnRender(EditorCamera& camera) {
    // ============ 1. 收集光照信息 ============

    LightEnvironment lightEnv;

    // 方向光
    {
        auto view = m_registry.view<TransformComponent, DirectionalLightComponent>();
        for (auto entity : view) {
            auto [transform, light] = view.get<TransformComponent, DirectionalLightComponent>(entity);

            glm::vec3 direction = glm::normalize(glm::mat3(transform.GetTransform()) * glm::vec3(0, -1, 0));
            lightEnv.SetDirectionalLight(direction, light.color, light.intensity);
            break;  // 只支持一个方向光
        }
    }

    // 点光源
    {
        auto view = m_registry.view<TransformComponent, PointLightComponent>();
        for (auto entity : view) {
            auto [transform, light] = view.get<TransformComponent, PointLightComponent>(entity);
            lightEnv.AddPointLight(transform.translation, light.color, light.intensity, light.radius);
        }
    }

    // ============ 2. 更新 Uniform Buffers ============

    auto& uboManager = RenderSystem::Get().GetUniformBufferManager();
    uboManager.UpdateCamera(camera);
    uboManager.UpdateLighting(lightEnv);

    // ============ 3. 渲染网格 ============

    {
        auto view = m_registry.view<TransformComponent, MeshRendererComponent>();

        for (auto entity : view) {
            auto [transform, meshRenderer] = view.get<TransformComponent, MeshRendererComponent>(entity);

            if (!meshRenderer.visible || !meshRenderer.mesh) continue;

            meshRenderer.mesh->Render(transform.GetTransform());
        }
    }

    // ============ 4. 渲染天空盒 ============

    // if (m_environment) {
    //     SkyboxRenderer::Render(m_environment, camera.GetViewProjection());
    // }
}
```

---

## 9. 开发阶段规划

### 9.1 完整开发路线图

```
┌─────────────────────────────────────────────────────────────────────┐
│                         Phase 0: 基础设施                            │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │  • 完善 Core 工具 (Ref, UUID, Logger, Assert)              │   │
│  │  • 建立项目结构和命名规范                                    │   │
│  └─────────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────────┘
                                   ↓
┌─────────────────────────────────────────────────────────────────────┐
│                    Phase 1: RHI 层完善 (当前)                        │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │  • UniformBuffer 实现                                       │   │
│  │  • Shader 反射完善                                          │   │
│  │  • Framebuffer 多附件支持                                   │   │
│  │  • Pipeline State 抽象 (Blend, DepthTest, Cull)            │   │
│  └─────────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────────┘
                                   ↓
┌─────────────────────────────────────────────────────────────────────┐
│                      Phase 2: 资源系统                               │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │  • Asset 基类和 AssetManager                                │   │
│  │  • Material 系统                                            │   │
│  │  • MeshResource / StaticMesh 分离                           │   │
│  │  • 材质库和 Shader 库整合                                    │   │
│  └─────────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────────┘
                                   ↓
┌─────────────────────────────────────────────────────────────────────┐
│                      Phase 3: RenderPass 系统                        │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │  • RenderPass 抽象和基类                                    │   │
│  │  • ForwardPass 实现                                         │   │
│  │  • SkyboxPass 实现                                          │   │
│  │  • 简单的 Pass 管理器                                       │   │
│  └─────────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────────┘
                                   ↓
┌─────────────────────────────────────────────────────────────────────┐
│                        Phase 4: 光照系统                             │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │  • 方向光 + 点光源 + 聚光灯                                  │   │
│  │  • Phong/Blinn-Phong 光照                                   │   │
│  │  • ShadowPass 和阴影映射                                    │   │
│  │  • PBR 材质和 IBL (可选)                                    │   │
│  └─────────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────────┘
                                   ↓
┌─────────────────────────────────────────────────────────────────────┐
│                        Phase 5: 场景系统                             │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │  • ECS 框架 (entt)                                          │   │
│  │  • 组件系统                                                  │   │
│  │  • 场景序列化 (YAML)                                        │   │
│  │  • 场景图/层级关系                                          │   │
│  └─────────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────────┘
                                   ↓
┌─────────────────────────────────────────────────────────────────────┐
│                       Phase 6: 编辑器基础                            │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │  • 场景层级面板                                             │   │
│  │  • 属性检视面板                                             │   │
│  │  • 视口渲染 (到 Framebuffer)                                │   │
│  │  • Gizmo (ImGuizmo)                                         │   │
│  │  • 内容浏览器                                               │   │
│  └─────────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────────┘
                                   ↓
┌─────────────────────────────────────────────────────────────────────┐
│                       Phase 7: 高级特性                              │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │  • 后处理系统 (Bloom, Tone Mapping, FXAA)                   │   │
│  │  • 骨骼动画                                                  │   │
│  │  • 粒子系统                                                  │   │
│  │  • 物理引擎集成                                             │   │
│  │  • 音频系统                                                  │   │
│  │  • 脚本系统 (Lua/C#)                                        │   │
│  └─────────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────────┘
```

### 9.2 各阶段详细任务

#### Phase 1: RHI 层完善（建议首先完成）

| 任务 | 优先级 | 依赖 | 说明 |
|------|--------|------|------|
| UniformBuffer | 最高 | 无 | GPU数据管理基础 |
| Shader 反射完善 | 高 | 无 | 支持 UBO 反射 |
| Pipeline State | 高 | 无 | 渲染状态管理 |
| Framebuffer 完善 | 中 | 无 | 多附件、MSAA |
| Sampler 对象 | 低 | 无 | 纹理采样配置 |

#### Phase 2: 资源系统

| 任务 | 优先级 | 依赖 | 说明 |
|------|--------|------|------|
| Asset 基类 | 高 | Phase 1 | 资源系统基础 |
| AssetManager | 高 | Asset | 资源加载管理 |
| Material 系统 | 最高 | Shader反射 | 材质数据绑定 |
| Mesh 重构 | 高 | Material | Resource/Instance |
| MaterialInstance | 中 | Material | 材质变体 |

#### Phase 3: RenderPass 系统

| 任务 | 优先级 | 依赖 | 说明 |
|------|--------|------|------|
| RenderPass 基类 | 最高 | Framebuffer | Pass 抽象 |
| ForwardPass | 高 | Material, Mesh | 前向渲染 |
| SkyboxPass | 中 | TextureCube | 天空盒渲染 |
| Pass 管理器 | 中 | RenderPass | Pass 调度 |

#### Phase 4: 光照系统

| 任务 | 优先级 | 依赖 | 说明 |
|------|--------|------|------|
| Light 数据结构 | 高 | UniformBuffer | 光源表示 |
| Phong 光照 | 高 | Light | 基础光照模型 |
| 阴影映射 | 中 | Framebuffer, RenderPass | 方向光阴影 |
| PBR 材质 | 低 | Material | 物理渲染 |
| IBL | 低 | TextureCube, PBR | 环境光照 |

### 9.3 学习资源推荐

**书籍**：
- 《Real-Time Rendering, 4th Edition》 - 渲染技术圣经
- 《Game Engine Architecture》 - 引擎架构全面指南
- 《GPU Gems》系列 - 高级渲染技术

**开源引擎参考**：
- **Hazel Engine** (TheCherno) - 最佳学习参考，代码清晰
- **Godot Engine** - 完整的2D/3D引擎
- **Piccolo Engine** (GAMES104) - 国产优秀教学引擎
- **Filament** (Google) - 移动端 PBR 渲染

**在线资源**：
- LearnOpenGL.com - OpenGL 基础到高级
- GAMES101/104/202 - 图形学课程
- GPU Gems Online - NVIDIA 渲染技术

---

## 附录：代码规范建议

### 命名约定

```cpp
// 类名: PascalCase
class RenderSystem;
class MaterialInstance;

// 函数名: PascalCase
void CreateEntity();
void OnUpdate();

// 成员变量: m_camelCase
Ref<Shader> m_shader;
uint32_t m_renderId;

// 静态成员: s_camelCase
static RenderSystem* s_instance;

// 常量: UPPER_CASE 或 k前缀
constexpr uint32_t MAX_LIGHTS = 16;
constexpr uint32_t kMaxBones = 100;

// 枚举值: PascalCase
enum class ShaderStage { Vertex, Fragment, Compute };
```

### 头文件组织

```cpp
// 1. 预编译头 (如果使用)
#include "precompile.h"

// 2. 对应的头文件 (cpp 文件专用)
#include "this_file.h"

// 3. 项目内头文件
#include "core/ref.h"
#include "render/shader.h"

// 4. 第三方库
#include <glm/glm.hpp>
#include <entt/entt.hpp>

// 5. 标准库
#include <string>
#include <vector>
#include <memory>
```

---

*本文档将随项目发展持续更新。*
