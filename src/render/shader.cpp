#include "shader.h"
#include "render/interface/opengl/opengl_shader.h"
#include "render/renderer_api.h"
#include "log_system.h"


namespace Mint {
    Ref<Shader> Shader::Create(const std::filesystem::path &filepath) {
        // Here we can add support for different rendering APIs in the future
        switch (RendererAPI::GetAPI()) {
            case RendererAPI::API::None:            return nullptr;
            case RendererAPI::API::OpenGL:          return Ref<OpenGLShader>::Create(filepath);
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