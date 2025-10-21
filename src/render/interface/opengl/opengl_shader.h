#pragma once
#include <string>

#include "../../gl_common.h"
#include "../../shader.h"


namespace Mint {
    class OpenGLShader : public Shader {
    public:
        OpenGLShader(const std::string& vertex_src, const std::string& fragment_src);
        ~OpenGLShader();

        void Bind() const;
        void Unbind() const;

        // temporary factory method
        void UploadUniformMat4(const std::string& name, const glm::mat4& matrix) override;

        static OpenGLShader* Create(const std::string& vertex_src, const std::string& fragment_src) {
            return new OpenGLShader(vertex_src, fragment_src);
        }
        // TODO: modify when reconstructing shader system


    private:
        uint32_t m_renderer_id;
    };
}