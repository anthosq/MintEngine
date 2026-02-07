#include "render/interface/opengl/opengl_material.h"
#include "log_system.h"

namespace Mint {

    OpenGLMaterial::OpenGLMaterial(const Ref<Shader>& shader, const std::string& name) 
        : m_Shader(shader), m_Name(name) {
            // Init();
            AllocateStorage();
            m_MaterialFlags |= (uint32_t)MaterialFlag::DepthTest;
            m_MaterialFlags |= (uint32_t)MaterialFlag::Blend;
    }

    OpenGLMaterial::OpenGLMaterial(Ref<Material>& other, const std::string& name) : m_Name(name) {
        Ref<OpenGLMaterial> glOther = other.As<OpenGLMaterial>();
        if (glOther) {
            m_Shader = glOther->m_Shader;
            m_MaterialFlags = glOther->m_MaterialFlags;
            
            if (glOther->m_UniformBufferData.Data) {
                m_UniformBufferData = Buffer::Copy(glOther->m_UniformBufferData.Data, glOther->m_UniformBufferData.Size);
            }

            AllocateStorage();

            m_BoundTextures = glOther->m_BoundTextures;
        }
    }

    void OpenGLMaterial::AllocateStorage() {
        const auto& shaderUniforms = m_Shader->GetUniforms();
        if (shaderUniforms.size() > 0) {
            uint32_t size = 0;
            for (auto & [name, uniform] : shaderUniforms) {
                if (uniform.GetOffset() != -1) { 
                    size = std::max(size, uniform.GetOffset() + uniform.GetSize());
                }
            }
            m_UniformBufferData.Allocate(size);
            m_UniformBufferData.ZeroInitialize();

            if (size > 0) {
                m_UniformBuffer = UniformBuffer::Create(size, 0);
            }
        }
    }


    OpenGLMaterial::~OpenGLMaterial() {

    }

    void OpenGLMaterial::Invalidate() {

    }

    void OpenGLMaterial::OnShaderReload() {

    }

    // OpenGLMaterial::Set Methods
    void OpenGLMaterial::Set(const std::string& name, float value) {
        Set<float>(name, value);
    }

    void OpenGLMaterial::Set(const std::string &name, int value) {
        Set<int>(name, value);
    }

    void OpenGLMaterial::Set(const std::string &name, uint32_t value) {
        Set<uint32_t>(name, value);
    }
    void OpenGLMaterial::Set(const std::string &name, bool value) {
        Set<bool>(name, value);
    }

    void OpenGLMaterial::Set(const std::string &name, const glm::vec2 &value) {
        Set<glm::vec2>(name, value);
    }

    void OpenGLMaterial::Set(const std::string &name, const glm::vec3 &value) {
        Set<glm::vec3>(name, value);
    }

    void OpenGLMaterial::Set(const std::string &name, const glm::vec4 &value) {
        Set<glm::vec4>(name, value);
    }

    void OpenGLMaterial::Set(const std::string &name, const glm::ivec2 &value) {
        Set<glm::ivec2>(name, value);
    }

    void OpenGLMaterial::Set(const std::string &name, const glm::ivec3 &value) {
        Set<glm::ivec3>(name, value);
    }

    void OpenGLMaterial::Set(const std::string &name, const glm::ivec4 &value) {
        Set<glm::ivec4>(name, value);
    }

    void OpenGLMaterial::Set(const std::string &name, const glm::mat3 &value) {
        Set<glm::mat3>(name, value);
    }

    void OpenGLMaterial::Set(const std::string &name, const glm::mat4 &value) {
        Set<glm::mat4>(name, value);
    }

    void OpenGLMaterial::Set(const std::string &name, const Ref<Texture2D> &texture) {
        auto decl = FindResourceInfo(name);
        if (!decl) {
            LOG_WARN(fmt::format("Texture2D resource not found: {0}", name));
            return;
        }
        uint32_t slot = decl->GetBindingPoint();
        // LOG_INFO(fmt::format("Texture2D set for resource: {0} at slot: {1}", name, slot));
        m_BoundTextures[slot] = texture;
        // LOG_INFO(fmt::format("Material::Set texture - BoundTextures size now: {}", m_BoundTextures.size()));
    }

    void OpenGLMaterial::Set(const std::string &name, const Ref<Texture2D> &texture, uint32_t arrayIndex) {
        auto decl = FindResourceInfo(name);
        if (!decl) {
            LOG_WARN(fmt::format("Texture2D array resource not found: {0}", name));
            return;
        }
        uint32_t slot = decl->GetBindingPoint() + arrayIndex;
        m_BoundTextures[slot] = texture;
    }

    void OpenGLMaterial::Set(const std::string &name, const Ref<TextureCube> &texture) {
        // Not sure
        auto decl = FindResourceInfo(name);
        if (!decl) {
            LOG_WARN(fmt::format("TextureCube resource not found: {0}", name));
            return;
        }
        uint32_t slot = decl->GetBindingPoint();
        // if (m_BoundTextures.size() <= slot)
        //     m_BoundTextures.resize((size_t)slot + 1);
        m_BoundTextures[slot] = texture;
    }

    void OpenGLMaterial::SetFlags(uint32_t flags) {}
    void OpenGLMaterial::SetFlag(MaterialFlag flag, bool value) {}

    float& OpenGLMaterial::GetFloat(const std::string &name) {
        return Get<float>(name);
    }
    int& OpenGLMaterial::GetInt(const std::string &name) {
        return Get<int>(name);
    }
    uint32_t& OpenGLMaterial::GetUInt(const std::string &name) {
        return Get<uint32_t>(name);
    }
    bool& OpenGLMaterial::GetBool(const std::string &name) {
        return Get<bool>(name);
    }

    glm::vec2& OpenGLMaterial::GetVec2(const std::string &name) {
        return Get<glm::vec2>(name);
    }
    glm::vec3& OpenGLMaterial::GetVec3(const std::string &name) {
        return Get<glm::vec3>(name);
    }
    glm::vec4& OpenGLMaterial::GetVec4(const std::string &name) {
        return Get<glm::vec4>(name);
    }

    glm::ivec2& OpenGLMaterial::GetIVec2(const std::string &name) {
        return Get<glm::ivec2>(name);
    }
    glm::ivec3& OpenGLMaterial::GetIVec3(const std::string &name) {
        return Get<glm::ivec3>(name);
    }
    glm::ivec4& OpenGLMaterial::GetIVec4(const std::string &name) {
        return Get<glm::ivec4>(name);
    }

    glm::mat3& OpenGLMaterial::GetMat3(const std::string &name) {
        return Get<glm::mat3>(name);
    }
    glm::mat4& OpenGLMaterial::GetMat4(const std::string &name) {
        return Get<glm::mat4>(name);
    }

    // TODO: Texture相关的实现还有问题, 应该实现一个基类Texture的模板

    Ref<Texture2D> OpenGLMaterial::GetTexture2D(const std::string &name) {
        auto decl = FindResourceInfo(name);
        if (!decl) {
            LOG_WARN(fmt::format("Texture2D resource not found: {0}", name));
            static Ref<Texture2D> nullTexture = nullptr;
            return nullTexture;
        }
        uint32_t slot = decl->GetBindingPoint();
        // Not sure
        auto it = m_BoundTextures.find(slot);
        if (it != m_BoundTextures.end()) {
            return m_BoundTextures[slot];
        } 
        // later use assert 来替换这里的分支判断
        else {
            LOG_WARN(fmt::format("Texture2D at slot {0} not set for resource: {1}", slot, name));
            static Ref<Texture2D> nullTexture = nullptr;
            return nullTexture;
        }
    }
    Ref<TextureCube> OpenGLMaterial::GetTextureCube(const std::string &name) {
        auto decl = FindResourceInfo(name);
        uint32_t slot = decl->GetBindingPoint();
        if (slot < m_BoundTextures.size()) {
            LOG_ERROR(fmt::format("TextureCube resource not found: {0}", name));
        }
        return m_BoundTextures[slot];
    }

    const ShaderUniform* OpenGLMaterial::FindUniformDeclaration(const std::string &name) const {
        const auto& uniforms = m_Shader->GetUniforms();
        auto it = uniforms.find(name);
        return it != uniforms.end() ? &it->second : nullptr;
    }

    const ShaderResourceInfo* OpenGLMaterial::FindResourceInfo(const std::string &name) const {
        auto& resources = m_Shader->GetResources();
        auto it = resources.find(name);
        return it != resources.end() ? &it->second : nullptr;
    }

    void OpenGLMaterial::Bind() {
        if (!m_Shader) {
            LOG_WARN("OpenGLMaterial::Bind: Shader is null");
            return;
        }
        // LOG_INFO(fmt::format("Shader: {}", m_Shader ? m_Shader->GetName() : "NULL"));
        // LOG_INFO(fmt::format("UniformBuffer: {}", m_UniformBuffer ? "EXISTS" : "NULL"));
        // LOG_INFO(fmt::format("UniformBufferData.Size: {}", m_UniformBufferData.Size));
        // LOG_INFO(fmt::format("BoundTextures.size: {}", m_BoundTextures.size()));

        m_Shader->Bind();
        // !!TODO: Use Dirty Flag later

        if (m_UniformBuffer) {
            m_UniformBuffer->SetData(m_UniformBufferData.Data, m_UniformBufferData.Size, 0);
        }

        // Texture
        for (auto && [slot, texture] : m_BoundTextures) {
            if (texture) {
                texture->Bind(slot);
            }
        }
    }
}