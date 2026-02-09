# MintEngine Code Review 问题追踪

> 审查日期: 2026-02-07
> 审查范围: 渲染系统、核心系统、编辑器层

---

## 问题统计

| 级别 | 数量 | 已修复 | 待修复 |
|------|------|--------|--------|
| Critical | 4 | 0 | 4 |
| Major | 6 | 0 | 6 |
| Minor | 10 | 0 | 10 |

---

## Critical 级别问题

### CR-001: RefCounter 非原子操作
- **状态**: [ ] 待修复
- **文件**: `src/core/ref.h:27-35`
- **影响**: 多线程环境下引用计数不准确，导致内存泄漏或二重释放

**问题代码**:
```cpp
void IncRefCount() const {
    // m_ref_count.fetch_add(1, std::memory_order_relaxed);  // 被注释掉！
    ++m_ref_count;  // 非原子操作
}

void DecRefCount() const {
    // m_ref_count.fetch_sub(1, std::memory_order_acq_rel);  // 被注释掉！
    --m_ref_count;  // 非原子操作
}
```

**解决方案**:
```cpp
void IncRefCount() const {
    m_ref_count.fetch_add(1, std::memory_order_relaxed);
}

void DecRefCount() const {
    m_ref_count.fetch_sub(1, std::memory_order_acq_rel);
}
```

---

### CR-002: RenderCommandQueue 缓冲区溢出
- **状态**: [ ] 待修复
- **文件**: `src/render/render_command_queue.cpp`
- **影响**: 缓冲区溢出导致堆损坏和程序崩溃

**问题代码**:
```cpp
m_CommandBuffer = new uint8_t[1024 * 1024 * 10];  // 10MB 固定大小

void* RenderCommandQueue::Allocate(RenderCommandFn func, uint32_t size) {
    // 无边界检查！
    *(RenderCommandFn*)m_CommandBufferPtr = func;
    m_CommandBufferPtr += sizeof(RenderCommandFn);
    // ...
}
```

**解决方案**:
```cpp
void* RenderCommandQueue::Allocate(RenderCommandFn func, uint32_t size) {
    constexpr uint32_t BUFFER_SIZE = 1024 * 1024 * 10;
    uint32_t needed = sizeof(RenderCommandFn) + sizeof(uint32_t) + size;
    uint32_t used = static_cast<uint32_t>(m_CommandBufferPtr - m_CommandBuffer);

    if (used + needed > BUFFER_SIZE) {
        LOG_ERROR("RenderCommandQueue buffer overflow! Used: {}, Needed: {}", used, needed);
        return nullptr;  // 或扩展缓冲区
    }

    // ... 继续分配
}
```

---

### CR-003: Buffer 类缺少 RAII
- **状态**: [ ] 待修复
- **文件**: `src/core/buffer.h`
- **影响**: 悬空指针、内存泄漏

**问题代码**:
```cpp
struct Buffer {
    byte* Data;
    uint32_t Size;

    Buffer() : Data(nullptr), Size(0) {}
    // 没有拷贝构造、移动构造、赋值操作符
    // 没有析构函数！
};
```

**解决方案**:
```cpp
struct Buffer {
    byte* Data = nullptr;
    uint32_t Size = 0;
    bool OwnsMemory = false;

    Buffer() = default;

    // 禁止复制
    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    // 启用移动
    Buffer(Buffer&& other) noexcept
        : Data(other.Data), Size(other.Size), OwnsMemory(other.OwnsMemory) {
        other.Data = nullptr;
        other.Size = 0;
        other.OwnsMemory = false;
    }

    Buffer& operator=(Buffer&& other) noexcept {
        if (this != &other) {
            Release();
            Data = other.Data;
            Size = other.Size;
            OwnsMemory = other.OwnsMemory;
            other.Data = nullptr;
            other.Size = 0;
            other.OwnsMemory = false;
        }
        return *this;
    }

    ~Buffer() {
        if (OwnsMemory) {
            Release();
        }
    }
};
```

---

### CR-004: 投影矩阵参数顺序错误
- **状态**: [ ] 待修复
- **文件**: `src/render/camera.h:29-30`
- **影响**: 投影矩阵计算错误，渲染异常

**问题代码**:
```cpp
void SetPerspectiveProjection(float deg_fov, float aspect_ratio, float zNear, float zFar) {
    m_projection_matrix = glm::perspective(glm::radians(deg_fov), aspect_ratio, zFar, zNear);
    //                                                                          ^^^^  ^^^^
    //                                                                    参数顺序颠倒！
}
```

**解决方案**:
```cpp
void SetPerspectiveProjection(float deg_fov, float aspect_ratio, float zNear, float zFar) {
    m_projection_matrix = glm::perspective(glm::radians(deg_fov), aspect_ratio, zNear, zFar);
}
```

---

## Major 级别问题

### CR-005: 单例内存泄漏
- **状态**: [ ] 待修复
- **文件**: `src/render/render_system.cpp:18, 23`
- **影响**: 程序退出时资源未释放

**问题代码**:
```cpp
RenderSystem* RenderSystem::m_renderer = new RenderSystem();      // 永不删除
RenderSystem::SceneData* RenderSystem::m_sceneData = new SceneData;  // 永不删除
```

**解决方案**:
```cpp
// 方案1: Meyer's Singleton
RenderSystem& RenderSystem::GetInstance() {
    static RenderSystem instance;
    return instance;
}

// 方案2: 使用 unique_ptr
static std::unique_ptr<RenderSystem> s_instance;
```

---

### CR-006: Uniform 位置未缓存
- **状态**: [ ] 待修复
- **文件**: `src/render/interface/opengl/opengl_shader.cpp:283-290`
- **影响**: 性能下降，每次设置 uniform 都调用 glGetUniformLocation

**问题代码**:
```cpp
void OpenGLShader::UploadUniformInt(const std::string& name, int value) {
    GLint Location = glGetUniformLocation(m_renderer_id, name.c_str());  // 每次都查询！
    glUniform1i(Location, value);
}
```

**解决方案**:
```cpp
// 在反射时缓存位置
std::unordered_map<std::string, GLint> m_uniformLocations;

void OpenGLShader::Reflect() {
    // ... 现有反射代码 ...
    m_uniformLocations[uniformName] = glGetUniformLocation(m_renderer_id, uniformName.c_str());
}

void OpenGLShader::UploadUniformInt(const std::string& name, int value) {
    auto it = m_uniformLocations.find(name);
    if (it != m_uniformLocations.end()) {
        glUniform1i(it->second, value);
    }
}
```

---

### CR-007: WeakRef 时间窗口问题
- **状态**: [ ] 待修复
- **文件**: `src/core/ref.h:211-233`
- **影响**: use-after-free 风险

**问题代码**:
```cpp
template <typename T>
class WeakRef {
public:
    bool IsValid() const {
        return m_instance ? RefUtils::IsAlive(m_instance) : false;
    }
    // IsAlive 检查和实际使用之间存在时间窗口！
};
```

**解决方案**:
```cpp
template <typename T>
class WeakRef {
public:
    // 提供安全的锁定方法
    Ref<T> Lock() const {
        std::lock_guard<std::mutex> lock(RefUtils::GetMutex());
        if (m_instance && RefUtils::IsAlive(m_instance)) {
            return Ref<T>(m_instance);  // 增加引用计数
        }
        return nullptr;
    }
};

// 使用方式
if (auto ptr = weakRef.Lock()) {
    // 安全使用 ptr
}
```

---

### CR-008: DecRef 二重释放风险
- **状态**: [ ] 待修复
- **文件**: `src/core/ref.h:189-200`
- **影响**: 二重释放导致程序崩溃

**问题代码**:
```cpp
void DecRef() const {
    if (m_instance) {
        m_instance->DecRefCount();
        // 会存在竞态吗？ <-- 代码注释已指出问题
        if (m_instance->GetRefCount() == 0) {
            delete std::exchange(m_instance, nullptr);
        }
    }
}
```

**解决方案**:
```cpp
// DecRefCount 应该返回新的引用计数
uint32_t RefCounter::DecRefCount() const {
    return m_ref_count.fetch_sub(1, std::memory_order_acq_rel) - 1;
}

void DecRef() const {
    if (m_instance) {
        uint32_t refCount = m_instance->DecRefCount();
        if (refCount == 0) {
            RefUtils::RemoveFromLiveRef((void*)m_instance);
            delete m_instance;
            m_instance = nullptr;
        }
    }
}
```

---

### CR-009: 深度测试配置不一致
- **状态**: [ ] 待修复
- **文件**: `src/render/interface/opengl/opengl_renderer_api.cpp:15-18`
- **影响**: 渲染异常

**问题代码**:
```cpp
glDepthFunc(GL_GREATER);  // 使用 Reverse-Z，但相机未配合
```

**说明**: 如果使用 Reverse-Z，需要：
1. 清除深度为 0.0f（而非 1.0f）
2. 投影矩阵需要反转 Z 范围
3. 确保所有相关代码一致

---

### CR-010: Material Get 返回引用
- **状态**: [ ] 待修复
- **文件**: `src/render/material.h:72-88`
- **影响**: 悬垂引用风险

**问题代码**:
```cpp
virtual float& GetFloat(const std::string& name) = 0;
virtual int& GetInt(const std::string& name) = 0;
// 代码注释: "我不认为这里应该传递引用"
```

**解决方案**:
```cpp
// 返回值而非引用
virtual float GetFloat(const std::string& name) const = 0;
virtual void SetFloat(const std::string& name, float value) = 0;
```

---

## Minor 级别问题

### CR-011: 命名规范不一致
- **状态**: [ ] 待修复
- **文件**: 多个文件
- **示例**:
  - `m_renderer_id` vs `m_RendererID`
  - `Get_Renderer()` 使用下划线

**建议**: 统一使用 camelCase 或 snake_case

---

### CR-012: Buffer 边界检查缺失
- **状态**: [ ] 待修复
- **文件**: `src/core/buffer.h:61-74`

**问题代码**:
```cpp
void Write(byte* data, uint32_t size, uint32_t offset = 0) {
    // ASSERT(offset + size <= Size, "Buffer overflow!");  // 被注释掉！
    memcpy(Data + offset, data, size);
}
```

---

### CR-013: EditorLayer 硬编码数据
- **状态**: [ ] 待修复（Mesh 系统将解决）
- **文件**: `MintEditor/src/editor_layer.cpp:6-58`
- **说明**: 顶点数据硬编码，应重构为 Mesh 系统

---

### CR-014: 注释代码未清理
- **状态**: [ ] 待修复
- **文件**: 多个文件
- **示例**:
  - `editor_layer.cpp:76-77` 注释掉的轮询代码
  - `editor_layer.cpp:343-361` 完全注释的 RenderTransparent

---

### CR-015: 全局静态对象初始化顺序
- **状态**: [ ] 待修复
- **文件**: `src/core/ref.cpp`, `src/uuid.cpp`
- **说明**: 跨编译单元的静态对象初始化顺序不确定

---

### CR-016: ShaderLibrary 使用 assert
- **状态**: [ ] 待修复
- **文件**: `src/render/shader.cpp:43-62`
- **说明**: Release 版本中 assert 被禁用，应使用运行时检查

---

### CR-017: OpenGLMaterial 拷贝构造不完整
- **状态**: [ ] 待修复
- **文件**: `src/render/interface/opengl/opengl_material.cpp:14-28`
- **说明**: 类型转换失败时对象处于半初始化状态

---

### CR-018: LayerStack 缺少异常安全
- **状态**: [ ] 待修复
- **文件**: `src/layer_stack.cpp:6-11`
- **说明**: OnDetach 抛出异常时后续 layer 不会被清理

---

### CR-019: ImGuiLayer 所有权不清晰
- **状态**: [ ] 待修复
- **文件**: `src/Application.cpp:24-25`
- **说明**: 裸指针管理，所有权语义不明确

---

### CR-020: Camera 宽高比不同步
- **状态**: [ ] 待修复
- **文件**: `MintEditor/src/editor_layer.cpp`
- **说明**: Camera 初始化宽高比与 Framebuffer/Viewport 不一致

---

## 修复进度追踪

### 阶段 0（Mesh 依赖）
- [ ] CR-003: Buffer RAII

### 阶段 1（Mesh 系统）
- [ ] CR-013: 硬编码数据重构

### 阶段 2（Critical 修复）
- [ ] CR-001: RefCounter 原子操作
- [ ] CR-002: RenderCommandQueue 边界检查
- [ ] CR-004: 投影矩阵参数

### 阶段 3（Major 修复）
- [ ] CR-005 ~ CR-010

### 阶段 4（Minor 修复）
- [ ] CR-011 ~ CR-020

---

## 修改记录

| 日期 | 问题ID | 操作 | 说明 |
|------|--------|------|------|
| 2026-02-07 | - | 创建 | 初始 Code Review |
