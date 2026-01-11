#include "shader.h"
#include "render/interface/opengl/opengl_shader.h"
#include "render/renderer_api.h"
#include "log_system.h"


namespace Mint {
    ShaderUniform::ShaderUniform(std::string name, ShaderUniformType type, uint32_t size, uint32_t offset)
        : m_name(std::move(name)), m_type(type), m_size(size), m_offset(offset) {}


    std::string_view ShaderUniform::UniformTypeToString(ShaderUniformType type) {
        switch (type) {
            case ShaderUniformType::None:    return "None";
            case ShaderUniformType::Bool:    return "Bool";
            case ShaderUniformType::Int:     return "Int";
            case ShaderUniformType::UInt:    return "UInt";
            case ShaderUniformType::Float:   return "Float";
            case ShaderUniformType::Vec2:    return "Vec2";
            case ShaderUniformType::Vec3:    return "Vec3";
            case ShaderUniformType::Vec4:    return "Vec4";
            case ShaderUniformType::Mat3:    return "Mat3";
            case ShaderUniformType::Mat4:    return "Mat4";
            case ShaderUniformType::IVec2:   return "IVec2";
            case ShaderUniformType::IVec3:   return "IVec3";
            case ShaderUniformType::IVec4:   return "IVec4";
            default:                         return "Unknown";
        }
    }


    Ref<Shader> Shader::Create(const std::filesystem::path &filepath) {
        // Here we can add support for different rendering APIs in the future
        switch (RendererAPI::GetAPIType()) {
            case RendererAPI::RenderAPIType::None:            return nullptr;
            case RendererAPI::RenderAPIType::OpenGL:          return Ref<OpenGLShader>::Create(filepath);
        }

        return nullptr;
    }

    // TODO: modify after completing ASSERT macro
    void ShaderLibrary::Add(const Ref<Shader> &shader) {
        std::string name = shader->GetName();
        assert(m_shaders.find(name) == m_shaders.end());
        m_shaders[name] = shader;
    }

    // TODO: use string_view
    void ShaderLibrary::Load(const std::string &name, const std::filesystem::path &filepath) {
        assert(m_shaders.find(name) == m_shaders.end());
        m_shaders[name] = Shader::Create(filepath);
    }

    void ShaderLibrary::Load(const std::filesystem::path &filepath) {
        std::string name = filepath.stem().string();
        Load(name, filepath);
    }

    const Ref<Shader> ShaderLibrary::Get(const std::string &name) const {
        assert(m_shaders.find(name) != m_shaders.end());
        return m_shaders.at(name);
    }


}