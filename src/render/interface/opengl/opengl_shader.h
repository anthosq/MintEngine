#pragma once
#include <string>

#include "render/interface/opengl/gl_common.h"
#include "render/shader.h"


namespace Mint {
    class OpenGLShader : public Shader {
    public:
        OpenGLShader(const std::filesystem::path& filepath);
        OpenGLShader(const std::string& vertex_src, const std::string& fragment_src);
        ~OpenGLShader();

        void Bind() const;
        void Unbind() const;


        // wrapper functions for shader
        void SetInt(const std::string& name, int value) override;

        void SetFloat(const std::string& name, float value) override;
        void SetFloat2(const std::string& name, const glm::vec2& vector) override;
        void SetFloat3(const std::string& name, const glm::vec3& vector) override;
        void SetFloat4(const std::string& name, const glm::vec4& vector) override;
        
        void SetMat3(const std::string& name, const glm::mat3& matrix) override;
        void SetMat4(const std::string& name, const glm::mat4& matrix) override;

    private:
        void UploadUniformInt(const std::string& name, int value);

        void UploadUniformFloat(const std::string& name, float value);
        void UploadUniformFloat2(const std::string& name, const glm::vec2& vector);
        void UploadUniformFloat3(const std::string& name, const glm::vec3& vector);
        void UploadUniformFloat4(const std::string& name, const glm::vec4& vector);

        void UploadUniformMat3(const std::string& name, const glm::mat3& matrix);
        void UploadUniformMat4(const std::string& name, const glm::mat4& matrix);

        void ReadShaderFromFile(const std::filesystem::path& filepath);
        static GLenum ShaderTypeFromString(const std::string& type);
        void CompileAndUploadShader();

    private:
        std::string m_name;
        std::filesystem::path m_asset_path;
        std::string m_shader_resource;
        uint32_t m_renderer_id;
    };

    // adding ShaderLibrary?
    class OpenGLShaderLibrary : public ShaderLibrary {
    public:
        virtual ~OpenGLShaderLibrary() = default;
        virtual void Add(const std::string& name, const std::shared_ptr<Shader>& shader) override;
        virtual Ref<Shader> Load(const std::string& name, const std::filesystem::path& filepath) override;
        virtual Ref<Shader> Load(const std::filesystem::path& filepath) override;
    };
}