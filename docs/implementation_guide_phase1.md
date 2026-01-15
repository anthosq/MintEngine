# MintEngine 实现指南 - Phase 1

> 本文档提供 UniformBuffer 和 Material 实现的完整代码与设计思路

---

## 目录

1. [总体设计思路](#1-总体设计思路)
2. [UniformBuffer 实现](#2-uniformbuffer-实现)
3. [OpenGLMaterial 实现](#3-openglmaterial-实现)
4. [Asset 基类完善](#4-asset-基类完善)
5. [集成测试](#5-集成测试)

---

# 1. 总体设计思路

## 1.1 数据流向

```
┌─────────────────────────────────────────────────────────────────────────┐
│                         Material 数据流                                  │
│                                                                          │
│   C++ 代码                     CPU Buffer                  GPU Buffer   │
│  ┌─────────┐                  ┌─────────┐                 ┌─────────┐   │
│  │material │  Set("color",   │ Uniform │   glBufferSub   │  UBO    │   │
│  │ .Set()  │ ─────────────▶  │ Buffer  │ ─────────────▶  │ (GPU)   │   │
│  └─────────┘   vec3(1,0,0))  │ (CPU)   │    Data()       └─────────┘   │
│                               └─────────┘                               │
│                                                                          │
│  关键点:                                                                 │
│  1. Material 持有 CPU 端数据副本                                         │
│  2. 数据变更时标记 dirty                                                 │
│  3. Bind() 时才真正上传到 GPU                                            │
└─────────────────────────────────────────────────────────────────────────┘
```

## 1.2 类关系图

```
┌─────────────────────────────────────────────────────────────────────────┐
│                                                                          │
│   Shader                                                                 │
│   ├── 反射信息 (Uniforms, UBOs)                                          │
│   └── ShaderBuffer (描述 UBO 结构)                                       │
│           │                                                              │
│           │ 读取结构信息                                                  │
│           ▼                                                              │
│   UniformBufferBase ◄─────────────── UniformBuffer<T>                   │
│   ├── m_data (CPU 数据)                                                  │
│   ├── m_rendererId (GPU UBO ID)                                         │
│   └── Upload() (上传到 GPU)                                              │
│           │                                                              │
│           │ 持有                                                          │
│           ▼                                                              │
│   Material                                                               │
│   ├── m_shader                                                          │
│   ├── m_uniformBuffer (材质参数 UBO)                                     │
│   ├── m_textures (纹理绑定)                                              │
│   └── Bind() (绑定 Shader + 上传数据)                                    │
│                                                                          │
└─────────────────────────────────────────────────────────────────────────┘
```

## 1.3 设计原则

1. **延迟上传**: 数据变更只修改 CPU 缓存，Bind() 时才上传 GPU
2. **脏标记**: 避免每帧重复上传未变化的数据
3. **反射驱动**: Material 根据 Shader 反射信息自动匹配参数
4. **类型安全**: 使用模板和类型检查防止错误赋值

---

# 2. UniformBuffer 实现

## 2.1 设计思路

UniformBuffer 需要解决几个问题：
1. **std140 布局**: GPU UBO 有特殊的内存对齐要求
2. **动态大小**: 不同 Shader 的 UBO 大小不同
3. **高效更新**: 支持部分更新，避免每次全量上传

## 2.2 std140 布局规则

```cpp
// std140 对齐规则（必须遵守，否则 GPU 读取数据错误）
//
// 类型           基础对齐    实际大小
// ─────────────────────────────────────
// float          4           4
// vec2           8           8
// vec3           16          12  (注意：对齐是16，但只占12字节)
// vec4           16          16
// mat3           16          48  (3个vec4，每行对齐到16)
// mat4           16          64
// float[]        16          每个元素对齐到16
// struct         16          整体对齐到16
//
// 常见陷阱：
// struct Light {
//     vec3 position;   // offset 0,  对齐 16
//     float intensity; // offset 12, 可以紧跟在 vec3 后面（因为 vec3 只占 12 字节）
//     vec3 color;      // offset 16, 新的 vec3 必须对齐到 16
//     float padding;   // offset 28, 填充到 32
// };
```

## 2.3 代码实现

### uniform_buffer.h

```cpp
#pragma once

#include "core/ref.h"
#include <cstdint>
#include <cstring>
#include <string>

namespace Mint {

    // ========== UniformBuffer 基类 ==========
    class UniformBufferBase : public RefCounter {
    public:
        virtual ~UniformBufferBase() = default;

        // 绑定到指定的 binding point
        virtual void Bind(uint32_t bindingPoint) const = 0;

        // 解绑
        virtual void Unbind() const = 0;

        // 上传全部数据到 GPU
        virtual void Upload() const = 0;

        // 上传部分数据（offset 和 size 以字节为单位）
        virtual void Upload(uint32_t offset, uint32_t size) const = 0;

        // 获取缓冲区大小
        virtual uint32_t GetSize() const = 0;

        // 获取原始数据指针（用于直接修改）
        virtual void* GetData() = 0;
        virtual const void* GetData() const = 0;

        // 设置数据（泛型接口）
        virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) = 0;

        // 标记为脏（需要重新上传）
        void SetDirty(bool dirty = true) { m_dirty = dirty; }
        bool IsDirty() const { return m_dirty; }

        // 创建工厂方法
        static Ref<UniformBufferBase> Create(uint32_t size);

    protected:
        mutable bool m_dirty = true;
    };

    // ========== 类型化 UniformBuffer ==========
    // 提供类型安全的访问方式
    template<typename T>
    class UniformBuffer : public UniformBufferBase {
    public:
        UniformBuffer() = default;
        virtual ~UniformBuffer() = default;

        // 获取类型化数据引用
        T& GetTypedData() {
            m_dirty = true;  // 假设调用者会修改
            return *reinterpret_cast<T*>(GetData());
        }

        const T& GetTypedData() const {
            return *reinterpret_cast<const T*>(GetData());
        }

        // 直接设置整个结构体
        void SetTypedData(const T& data) {
            std::memcpy(GetData(), &data, sizeof(T));
            m_dirty = true;
        }

        // 工厂方法
        static Ref<UniformBuffer<T>> Create() {
            auto base = UniformBufferBase::Create(sizeof(T));
            // 注意：这里需要返回正确的类型，实际实现中可能需要调整
            return Ref<UniformBuffer<T>>(static_cast<UniformBuffer<T>*>(base.get()));
        }
    };

    // ========== 常用的标准 Uniform 结构体 ==========

    // 相机数据 (binding point 0)
    struct CameraData {
        glm::mat4 viewProjection;      // offset 0
        glm::mat4 view;                 // offset 64
        glm::mat4 projection;           // offset 128
        glm::vec3 position;             // offset 192
        float padding1;                 // offset 204
    }; // size: 208

    // 变换数据 (binding point 1)
    struct TransformData {
        glm::mat4 model;                // offset 0
        glm::mat4 normalMatrix;         // offset 64 (用 mat4 存 mat3，便于对齐)
    }; // size: 128

    // 光照数据 (binding point 3)
    struct DirectionalLightData {
        glm::vec3 direction;            // offset 0
        float intensity;                // offset 12
        glm::vec3 color;                // offset 16
        float padding;                  // offset 28
    }; // size: 32

    struct PointLightData {
        glm::vec3 position;             // offset 0
        float intensity;                // offset 12
        glm::vec3 color;                // offset 16
        float radius;                   // offset 28
        float falloff;                  // offset 32
        float padding[3];               // offset 36
    }; // size: 48

    // 场景光照数据
    static constexpr uint32_t MAX_POINT_LIGHTS = 16;

    struct SceneLightingData {
        DirectionalLightData directionalLight;              // offset 0
        PointLightData pointLights[MAX_POINT_LIGHTS];       // offset 32
        uint32_t pointLightCount;                           // offset 32 + 48*16 = 800
        float ambientIntensity;                             // offset 804
        glm::vec2 padding;                                  // offset 808
    }; // size: 816

    // ========== Binding Points 常量 ==========
    namespace UniformBindingPoints {
        constexpr uint32_t Camera = 0;
        constexpr uint32_t Transform = 1;
        constexpr uint32_t Material = 2;
        constexpr uint32_t Lighting = 3;
        constexpr uint32_t Environment = 4;
        constexpr uint32_t Skinning = 5;
    }

} // namespace Mint
```

### interface/opengl/opengl_uniform_buffer.h

```cpp
#pragma once

#include "render/uniform_buffer.h"
#include <vector>

namespace Mint {

    class OpenGLUniformBuffer : public UniformBufferBase {
    public:
        OpenGLUniformBuffer(uint32_t size);
        virtual ~OpenGLUniformBuffer();

        // 禁止拷贝
        OpenGLUniformBuffer(const OpenGLUniformBuffer&) = delete;
        OpenGLUniformBuffer& operator=(const OpenGLUniformBuffer&) = delete;

        // 实现基类接口
        void Bind(uint32_t bindingPoint) const override;
        void Unbind() const override;
        void Upload() const override;
        void Upload(uint32_t offset, uint32_t size) const override;

        uint32_t GetSize() const override { return m_size; }
        void* GetData() override { return m_data.data(); }
        const void* GetData() const override { return m_data.data(); }
        void SetData(const void* data, uint32_t size, uint32_t offset = 0) override;

    private:
        uint32_t m_rendererId = 0;      // OpenGL UBO ID
        uint32_t m_size = 0;            // 缓冲区大小
        std::vector<uint8_t> m_data;    // CPU 端数据副本
        mutable uint32_t m_boundBindingPoint = 0;
    };

} // namespace Mint
```

### interface/opengl/opengl_uniform_buffer.cpp

```cpp
#include "opengl_uniform_buffer.h"
#include <glad/glad.h>
#include <cstring>

namespace Mint {

    OpenGLUniformBuffer::OpenGLUniformBuffer(uint32_t size)
        : m_size(size)
    {
        // 分配 CPU 端内存
        m_data.resize(size, 0);

        // 创建 GPU 端 UBO
        glCreateBuffers(1, &m_rendererId);

        // 分配 GPU 内存（使用 DYNAMIC_DRAW 因为可能频繁更新）
        glNamedBufferData(m_rendererId, size, nullptr, GL_DYNAMIC_DRAW);
    }

    OpenGLUniformBuffer::~OpenGLUniformBuffer() {
        glDeleteBuffers(1, &m_rendererId);
    }

    void OpenGLUniformBuffer::Bind(uint32_t bindingPoint) const {
        glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, m_rendererId);
        m_boundBindingPoint = bindingPoint;

        // 如果数据有变化，自动上传
        if (m_dirty) {
            Upload();
        }
    }

    void OpenGLUniformBuffer::Unbind() const {
        glBindBufferBase(GL_UNIFORM_BUFFER, m_boundBindingPoint, 0);
    }

    void OpenGLUniformBuffer::Upload() const {
        if (!m_dirty) return;

        glNamedBufferSubData(m_rendererId, 0, m_size, m_data.data());
        m_dirty = false;
    }

    void OpenGLUniformBuffer::Upload(uint32_t offset, uint32_t size) const {
        // 边界检查
        if (offset + size > m_size) {
            MINT_CORE_ERROR("UniformBuffer::Upload - Out of bounds: offset={}, size={}, bufferSize={}",
                           offset, size, m_size);
            return;
        }

        glNamedBufferSubData(m_rendererId, offset, size, m_data.data() + offset);
    }

    void OpenGLUniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset) {
        // 边界检查
        if (offset + size > m_size) {
            MINT_CORE_ERROR("UniformBuffer::SetData - Out of bounds");
            return;
        }

        std::memcpy(m_data.data() + offset, data, size);
        m_dirty = true;
    }

    // 工厂方法实现
    Ref<UniformBufferBase> UniformBufferBase::Create(uint32_t size) {
        return CreateRef<OpenGLUniformBuffer>(size);
    }

} // namespace Mint
```

---

# 3. OpenGLMaterial 实现

## 3.1 设计思路

Material 的职责：
1. 持有 Shader 引用
2. 管理材质参数（存储在 CPU 端 buffer）
3. 管理纹理绑定
4. Bind() 时上传数据到 GPU

关键设计决策：
- **使用字节数组存储参数**: 灵活支持任意类型
- **基于 Shader 反射自动布局**: 参数位置由 Shader 决定
- **纹理单独管理**: 纹理绑定和 Uniform 参数分开处理

## 3.2 代码实现

### interface/opengl/opengl_material.h

```cpp
#pragma once

#include "render/material.h"
#include "render/uniform_buffer.h"
#include <vector>
#include <unordered_map>

namespace Mint {

    class OpenGLMaterial : public Material {
    public:
        OpenGLMaterial(const Ref<Shader>& shader, const std::string& name);
        virtual ~OpenGLMaterial() = default;

        // 从另一个 Material 复制
        static Ref<OpenGLMaterial> Copy(const Ref<OpenGLMaterial>& other, const std::string& name);

        // ========== 生命周期 ==========
        void Invalidate() override;
        void OnShaderReload() override;

        // ========== Flags ==========
        void SetFlags(uint32_t flags) override { m_flags = flags; }
        void SetFlag(MaterialFlag flag, bool value = true) override;
        uint32_t GetFlags() const override { return m_flags; }
        bool GetFlag(MaterialFlag flag) const override;

        // ========== Setters ==========
        // Scalar
        void Set(const std::string& name, float value) override;
        void Set(const std::string& name, int value) override;
        void Set(const std::string& name, uint32_t value) override;
        void Set(const std::string& name, bool value) override;

        // Vector
        void Set(const std::string& name, const glm::vec2& value) override;
        void Set(const std::string& name, const glm::vec3& value) override;
        void Set(const std::string& name, const glm::vec4& value) override;
        void Set(const std::string& name, const glm::ivec2& value) override;
        void Set(const std::string& name, const glm::ivec3& value) override;
        void Set(const std::string& name, const glm::ivec4& value) override;

        // Matrix
        void Set(const std::string& name, const glm::mat3& value) override;
        void Set(const std::string& name, const glm::mat4& value) override;

        // Texture
        void Set(const std::string& name, const Ref<Texture2D>& texture) override;
        void Set(const std::string& name, const Ref<Texture2D>& texture, uint32_t arrayIndex) override;
        void Set(const std::string& name, const Ref<TextureCube>& texture) override;

        // ========== Getters ==========
        Ref<Shader> GetShader() const override { return m_shader; }
        const std::string& GetName() const override { return m_name; }

        // Scalar
        float& GetFloat(const std::string& name) const override;
        int& GetInt(const std::string& name) const override;
        uint32_t& GetUInt(const std::string& name) const override;
        bool& GetBool(const std::string& name) const override;

        // Vector
        glm::vec2& GetVec2(const std::string& name) const override;
        glm::vec3& GetVec3(const std::string& name) const override;
        glm::vec4& GetVec4(const std::string& name) const override;
        glm::ivec2& GetIVec2(const std::string& name) const override;
        glm::ivec3& GetIVec3(const std::string& name) const override;
        glm::ivec4& GetIVec4(const std::string& name) const override;

        // Matrix
        glm::mat3& GetMat3(const std::string& name) const override;
        glm::mat4& GetMat4(const std::string& name) const override;

        // Texture
        Ref<Texture2D>& GetTexture2D(const std::string& name) const override;
        Ref<TextureCube>& GetTextureCube(const std::string& name) const override;

        // ========== 渲染 ==========
        // 绑定材质（Shader + Uniforms + Textures）
        void Bind();

        // 仅上传 Uniform 数据（不绑定 Shader）
        void UploadUniforms();

    private:
        // 根据名称获取 Uniform 的偏移和大小
        bool GetUniformLocation(const std::string& name, uint32_t& offset, uint32_t& size) const;

        // 设置原始数据
        void SetUniformData(const std::string& name, const void* data, uint32_t size);

        // 获取原始数据指针
        void* GetUniformData(const std::string& name) const;

        // 分配 Uniform Buffer
        void AllocateUniformBuffer();

    private:
        std::string m_name;
        Ref<Shader> m_shader;
        uint32_t m_flags = 0;

        // Uniform 数据存储
        // 使用原始字节数组存储所有 uniform 值
        mutable std::vector<uint8_t> m_uniformData;

        // Uniform Buffer (用于上传到 GPU)
        Ref<UniformBufferBase> m_uniformBuffer;

        // Uniform 元数据缓存 (从 Shader 反射获取)
        struct UniformInfo {
            uint32_t offset;
            uint32_t size;
            ShaderUniformType type;
        };
        std::unordered_map<std::string, UniformInfo> m_uniformLocations;

        // 纹理绑定
        struct TextureBinding {
            Ref<Texture2D> texture2D;
            Ref<TextureCube> textureCube;
            uint32_t slot;
        };
        mutable std::unordered_map<std::string, TextureBinding> m_textures;

        // 脏标记
        mutable bool m_uniformsDirty = true;
    };

} // namespace Mint
```

### interface/opengl/opengl_material.cpp

```cpp
#include "opengl_material.h"
#include "core/log.h"
#include <cstring>

namespace Mint {

    // ========== 工厂方法 ==========

    Ref<Material> Material::Create(const Ref<Shader>& shader, const std::string& name) {
        return CreateRef<OpenGLMaterial>(shader, name);
    }

    Ref<Material> Material::Copy(const Ref<Material>& other, const std::string& name) {
        auto glMaterial = std::dynamic_pointer_cast<OpenGLMaterial>(other);
        if (!glMaterial) {
            MINT_CORE_ERROR("Material::Copy - Invalid material type");
            return nullptr;
        }
        return OpenGLMaterial::Copy(glMaterial, name);
    }

    // ========== 构造与初始化 ==========

    OpenGLMaterial::OpenGLMaterial(const Ref<Shader>& shader, const std::string& name)
        : m_name(name.empty() ? shader->GetName() + "_Material" : name)
        , m_shader(shader)
    {
        // 初始化 Uniform Buffer
        AllocateUniformBuffer();
    }

    Ref<OpenGLMaterial> OpenGLMaterial::Copy(const Ref<OpenGLMaterial>& other, const std::string& name) {
        auto material = CreateRef<OpenGLMaterial>(other->m_shader, name);

        // 复制 uniform 数据
        material->m_uniformData = other->m_uniformData;
        material->m_flags = other->m_flags;

        // 复制纹理绑定
        material->m_textures = other->m_textures;

        material->m_uniformsDirty = true;
        return material;
    }

    void OpenGLMaterial::AllocateUniformBuffer() {
        // 从 Shader 反射获取 Uniform 信息
        const auto& uniforms = m_shader->GetUniforms();

        if (uniforms.empty()) {
            return;
        }

        // 计算总大小和偏移
        uint32_t totalSize = 0;
        for (const auto& [name, uniform] : uniforms) {
            UniformInfo info;
            info.offset = uniform.GetOffset();
            info.size = uniform.GetSize();
            info.type = uniform.GetType();

            m_uniformLocations[name] = info;

            // 更新总大小
            uint32_t end = info.offset + info.size;
            if (end > totalSize) {
                totalSize = end;
            }
        }

        // 分配 CPU 端存储
        m_uniformData.resize(totalSize, 0);

        // 创建 GPU 端 UBO
        if (totalSize > 0) {
            m_uniformBuffer = UniformBufferBase::Create(totalSize);
        }
    }

    void OpenGLMaterial::Invalidate() {
        m_uniformsDirty = true;
    }

    void OpenGLMaterial::OnShaderReload() {
        // Shader 重新编译后，需要重新获取 uniform 位置
        m_uniformLocations.clear();
        AllocateUniformBuffer();
        m_uniformsDirty = true;
    }

    // ========== Flags ==========

    void OpenGLMaterial::SetFlag(MaterialFlag flag, bool value) {
        if (value) {
            m_flags |= static_cast<uint32_t>(flag);
        } else {
            m_flags &= ~static_cast<uint32_t>(flag);
        }
    }

    bool OpenGLMaterial::GetFlag(MaterialFlag flag) const {
        return (m_flags & static_cast<uint32_t>(flag)) != 0;
    }

    // ========== Uniform 数据操作 ==========

    bool OpenGLMaterial::GetUniformLocation(const std::string& name, uint32_t& offset, uint32_t& size) const {
        auto it = m_uniformLocations.find(name);
        if (it == m_uniformLocations.end()) {
            MINT_CORE_WARN("Material '{}': Uniform '{}' not found", m_name, name);
            return false;
        }
        offset = it->second.offset;
        size = it->second.size;
        return true;
    }

    void OpenGLMaterial::SetUniformData(const std::string& name, const void* data, uint32_t size) {
        uint32_t offset, expectedSize;
        if (!GetUniformLocation(name, offset, expectedSize)) {
            return;
        }

        if (size != expectedSize) {
            MINT_CORE_WARN("Material '{}': Size mismatch for uniform '{}' (expected {}, got {})",
                          m_name, name, expectedSize, size);
        }

        std::memcpy(m_uniformData.data() + offset, data, std::min(size, expectedSize));
        m_uniformsDirty = true;
    }

    void* OpenGLMaterial::GetUniformData(const std::string& name) const {
        uint32_t offset, size;
        if (!GetUniformLocation(name, offset, size)) {
            static float dummy = 0.0f;
            return &dummy;
        }
        return const_cast<uint8_t*>(m_uniformData.data() + offset);
    }

    // ========== Setters ==========

    void OpenGLMaterial::Set(const std::string& name, float value) {
        SetUniformData(name, &value, sizeof(float));
    }

    void OpenGLMaterial::Set(const std::string& name, int value) {
        SetUniformData(name, &value, sizeof(int));
    }

    void OpenGLMaterial::Set(const std::string& name, uint32_t value) {
        SetUniformData(name, &value, sizeof(uint32_t));
    }

    void OpenGLMaterial::Set(const std::string& name, bool value) {
        int intValue = value ? 1 : 0;  // GLSL bool 通常映射为 int
        SetUniformData(name, &intValue, sizeof(int));
    }

    void OpenGLMaterial::Set(const std::string& name, const glm::vec2& value) {
        SetUniformData(name, &value, sizeof(glm::vec2));
    }

    void OpenGLMaterial::Set(const std::string& name, const glm::vec3& value) {
        SetUniformData(name, &value, sizeof(glm::vec3));
    }

    void OpenGLMaterial::Set(const std::string& name, const glm::vec4& value) {
        SetUniformData(name, &value, sizeof(glm::vec4));
    }

    void OpenGLMaterial::Set(const std::string& name, const glm::ivec2& value) {
        SetUniformData(name, &value, sizeof(glm::ivec2));
    }

    void OpenGLMaterial::Set(const std::string& name, const glm::ivec3& value) {
        SetUniformData(name, &value, sizeof(glm::ivec3));
    }

    void OpenGLMaterial::Set(const std::string& name, const glm::ivec4& value) {
        SetUniformData(name, &value, sizeof(glm::ivec4));
    }

    void OpenGLMaterial::Set(const std::string& name, const glm::mat3& value) {
        SetUniformData(name, &value, sizeof(glm::mat3));
    }

    void OpenGLMaterial::Set(const std::string& name, const glm::mat4& value) {
        SetUniformData(name, &value, sizeof(glm::mat4));
    }

    void OpenGLMaterial::Set(const std::string& name, const Ref<Texture2D>& texture) {
        Set(name, texture, 0);
    }

    void OpenGLMaterial::Set(const std::string& name, const Ref<Texture2D>& texture, uint32_t arrayIndex) {
        auto& binding = m_textures[name];
        binding.texture2D = texture;
        binding.textureCube = nullptr;
        // slot 会在 Bind() 时根据 Shader 反射确定
    }

    void OpenGLMaterial::Set(const std::string& name, const Ref<TextureCube>& texture) {
        auto& binding = m_textures[name];
        binding.texture2D = nullptr;
        binding.textureCube = texture;
    }

    // ========== Getters ==========

    float& OpenGLMaterial::GetFloat(const std::string& name) const {
        return *reinterpret_cast<float*>(GetUniformData(name));
    }

    int& OpenGLMaterial::GetInt(const std::string& name) const {
        return *reinterpret_cast<int*>(GetUniformData(name));
    }

    uint32_t& OpenGLMaterial::GetUInt(const std::string& name) const {
        return *reinterpret_cast<uint32_t*>(GetUniformData(name));
    }

    bool& OpenGLMaterial::GetBool(const std::string& name) const {
        return *reinterpret_cast<bool*>(GetUniformData(name));
    }

    glm::vec2& OpenGLMaterial::GetVec2(const std::string& name) const {
        return *reinterpret_cast<glm::vec2*>(GetUniformData(name));
    }

    glm::vec3& OpenGLMaterial::GetVec3(const std::string& name) const {
        return *reinterpret_cast<glm::vec3*>(GetUniformData(name));
    }

    glm::vec4& OpenGLMaterial::GetVec4(const std::string& name) const {
        return *reinterpret_cast<glm::vec4*>(GetUniformData(name));
    }

    glm::ivec2& OpenGLMaterial::GetIVec2(const std::string& name) const {
        return *reinterpret_cast<glm::ivec2*>(GetUniformData(name));
    }

    glm::ivec3& OpenGLMaterial::GetIVec3(const std::string& name) const {
        return *reinterpret_cast<glm::ivec3*>(GetUniformData(name));
    }

    glm::ivec4& OpenGLMaterial::GetIVec4(const std::string& name) const {
        return *reinterpret_cast<glm::ivec4*>(GetUniformData(name));
    }

    glm::mat3& OpenGLMaterial::GetMat3(const std::string& name) const {
        return *reinterpret_cast<glm::mat3*>(GetUniformData(name));
    }

    glm::mat4& OpenGLMaterial::GetMat4(const std::string& name) const {
        return *reinterpret_cast<glm::mat4*>(GetUniformData(name));
    }

    Ref<Texture2D>& OpenGLMaterial::GetTexture2D(const std::string& name) const {
        static Ref<Texture2D> nullTexture;
        auto it = m_textures.find(name);
        if (it == m_textures.end()) {
            return nullTexture;
        }
        return it->second.texture2D;
    }

    Ref<TextureCube>& OpenGLMaterial::GetTextureCube(const std::string& name) const {
        static Ref<TextureCube> nullTexture;
        auto it = m_textures.find(name);
        if (it == m_textures.end()) {
            return nullTexture;
        }
        return it->second.textureCube;
    }

    // ========== 渲染 ==========

    void OpenGLMaterial::Bind() {
        // 1. 绑定 Shader
        m_shader->Bind();

        // 2. 上传 Uniform 数据
        UploadUniforms();

        // 3. 绑定纹理
        uint32_t textureSlot = 0;
        for (auto& [name, binding] : m_textures) {
            if (binding.texture2D) {
                binding.texture2D->Bind(textureSlot);
                m_shader->SetInt(name, textureSlot);
                textureSlot++;
            } else if (binding.textureCube) {
                binding.textureCube->Bind(textureSlot);
                m_shader->SetInt(name, textureSlot);
                textureSlot++;
            }
        }

        // 4. 应用材质标志（深度测试、混合等）
        // 这部分可以由 RenderState 管理，这里简化处理
    }

    void OpenGLMaterial::UploadUniforms() {
        if (!m_uniformsDirty || !m_uniformBuffer) {
            return;
        }

        // 将 CPU 数据复制到 UniformBuffer
        m_uniformBuffer->SetData(m_uniformData.data(), m_uniformData.size());

        // 绑定到材质 binding point
        m_uniformBuffer->Bind(UniformBindingPoints::Material);

        m_uniformsDirty = false;
    }

} // namespace Mint
```

---

# 4. Asset 基类完善

## 4.1 设计思路

Asset 系统需要：
1. 唯一标识（UUID）
2. 资源路径
3. 资源状态（加载中、已加载、失败）
4. 热重载支持

## 4.2 代码实现

### asset.h

```cpp
#pragma once

#include "core/ref.h"
#include "uuid.h"
#include <filesystem>
#include <string>

namespace Mint {

    // 资源类型枚举
    enum class AssetType : uint16_t {
        None = 0,
        Texture2D,
        TextureCube,
        Shader,
        Material,
        Mesh,
        Animation,
        Audio,
        Scene,
        Prefab,
        Script,
        Font
    };

    // 资源状态
    enum class AssetState : uint8_t {
        NotLoaded,
        Loading,
        Loaded,
        Failed
    };

    // 资源元数据
    struct AssetMetadata {
        UUID uuid;
        AssetType type = AssetType::None;
        std::filesystem::path filepath;

        bool IsValid() const { return uuid != 0 && type != AssetType::None; }
    };

    // 资源基类
    class Asset : public RefCounter {
    public:
        virtual ~Asset() = default;

        // 类型信息
        virtual AssetType GetType() const = 0;
        static AssetType GetStaticType() { return AssetType::None; }

        // 唯一标识
        UUID GetUUID() const { return m_uuid; }
        void SetUUID(UUID uuid) { m_uuid = uuid; }

        // 文件路径
        const std::filesystem::path& GetFilePath() const { return m_filepath; }
        void SetFilePath(const std::filesystem::path& path) { m_filepath = path; }

        // 名称（通常是文件名）
        const std::string& GetName() const { return m_name; }
        void SetName(const std::string& name) { m_name = name; }

        // 状态
        AssetState GetState() const { return m_state; }
        bool IsLoaded() const { return m_state == AssetState::Loaded; }

        // 热重载
        virtual void Reload() {}

        // 判断是否有效
        bool IsValid() const { return m_uuid != 0; }

    protected:
        UUID m_uuid;
        std::filesystem::path m_filepath;
        std::string m_name;
        AssetState m_state = AssetState::NotLoaded;
    };

    // ========== 类型安全的 Asset 转换 ==========

    template<typename T>
    Ref<T> AssetAs(const Ref<Asset>& asset) {
        if (asset && asset->GetType() == T::GetStaticType()) {
            return std::static_pointer_cast<T>(asset);
        }
        return nullptr;
    }

    // ========== Asset 类型注册宏 ==========

    #define ASSET_TYPE(type) \
        static AssetType GetStaticType() { return AssetType::type; } \
        virtual AssetType GetType() const override { return GetStaticType(); }

} // namespace Mint
```

## 4.3 在现有类中使用 Asset

```cpp
// 在 Texture2D 中
class Texture2D : public Asset {
public:
    ASSET_TYPE(Texture2D)
    // ...
};

// 在 Shader 中
class Shader : public Asset {
public:
    ASSET_TYPE(Shader)
    // ...
};

// 在 Material 中
class Material : public Asset {
public:
    ASSET_TYPE(Material)
    // ...
};
```

---

# 5. 集成测试

## 5.1 测试用例

```cpp
// 在 Sandbox 或测试代码中

void TestMaterialSystem() {
    // 1. 创建 Shader
    auto shader = Shader::Create("assets/shaders/pbr.glsl");

    // 2. 创建 Material
    auto material = Material::Create(shader, "TestMaterial");

    // 3. 设置参数
    material->Set("u_Albedo", glm::vec3(1.0f, 0.5f, 0.3f));
    material->Set("u_Metallic", 0.5f);
    material->Set("u_Roughness", 0.3f);

    // 4. 设置纹理
    auto albedoTex = Texture2D::Create("assets/textures/brick_albedo.png");
    material->Set("u_AlbedoMap", albedoTex);

    // 5. 渲染时绑定
    material->Bind();  // 自动绑定 Shader + 上传 Uniforms + 绑定纹理

    // 6. 绘制 Mesh
    mesh->Draw();
}

// 对应的 PBR Shader (pbr.glsl)
/*
#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

layout(std140, binding = 0) uniform Camera {
    mat4 u_ViewProjection;
    mat4 u_View;
    mat4 u_Projection;
    vec3 u_CameraPosition;
};

layout(std140, binding = 1) uniform Transform {
    mat4 u_Model;
    mat4 u_NormalMatrix;
};

out vec3 v_WorldPos;
out vec3 v_Normal;
out vec2 v_TexCoord;

void main() {
    v_WorldPos = vec3(u_Model * vec4(a_Position, 1.0));
    v_Normal = mat3(u_NormalMatrix) * a_Normal;
    v_TexCoord = a_TexCoord;
    gl_Position = u_ViewProjection * vec4(v_WorldPos, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

in vec3 v_WorldPos;
in vec3 v_Normal;
in vec2 v_TexCoord;

// Material uniforms (binding = 2)
layout(std140, binding = 2) uniform Material {
    vec3 u_Albedo;
    float u_Metallic;
    float u_Roughness;
    float u_AO;
};

uniform sampler2D u_AlbedoMap;
uniform sampler2D u_NormalMap;
uniform sampler2D u_MetallicMap;
uniform sampler2D u_RoughnessMap;

void main() {
    vec3 albedo = texture(u_AlbedoMap, v_TexCoord).rgb * u_Albedo;
    // ... PBR 计算
    o_Color = vec4(albedo, 1.0);
}
*/
```

## 5.2 验证清单

```
□ UniformBuffer 创建和销毁正常
□ 数据上传到 GPU 正确（使用 RenderDoc 验证）
□ Material::Set 各类型都能正常工作
□ Material::Get 返回正确的引用
□ 纹理绑定正确
□ Shader 反射信息正确读取
□ 脏标记机制正常工作（避免重复上传）
□ Material::Copy 能正确复制数据
```

---

# 附录: 文件清单

实现完成后的文件结构：

```
src/
├── uuid.h                    ✅ 已完成
├── uuid.cpp                  ✅ 已完成
├── asset.h                   📝 需要更新
├── render/
│   ├── uniform_buffer.h      📝 新建
│   ├── material.h            ✅ 已完成
│   ├── material.cpp          📝 新建
│   └── interface/
│       └── opengl/
│           ├── opengl_uniform_buffer.h    📝 新建
│           ├── opengl_uniform_buffer.cpp  📝 新建
│           ├── opengl_material.h          📝 新建
│           └── opengl_material.cpp        📝 新建
```

---

*文档版本: 1.0*
*创建日期: 2026-01-15*
