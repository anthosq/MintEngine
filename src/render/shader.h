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

        // wrapper functions
        virtual void SetInt(const std::string& name, int value) = 0;

        virtual void SetFloat(const std::string& name, float value) = 0;
        virtual void SetFloat2(const std::string& name, const glm::vec2& vector) = 0;
        virtual void SetFloat3(const std::string& name, const glm::vec3& vector) = 0;
        virtual void SetFloat4(const std::string& name, const glm::vec4& vector) = 0;

        virtual void SetMat3(const std::string& name, const glm::mat3& matrix) = 0;
        virtual void SetMat4(const std::string& name, const glm::mat4& matrix) = 0;
        


        static Shader* Create(const std::string& vertex_src, const std::string& fragment_src);
    };
}