#pragma once

#include <string>
#include <cstdint>
#include <memory>
#include <glm/glm.hpp>

namespace Mint {
    class Shader {
    public:
        // temporary factory method
        virtual ~Shader() = default;

        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;

        virtual void UploadUniformMat4(const std::string& name, const glm::mat4& matrix) = 0;

        static Shader* Create(const std::string& vertex_src, const std::string& fragment_src);
    };
}