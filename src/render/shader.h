#pragma once

#include <string>
#include <cstdint>
#include <memory>
#include <glm/glm.hpp>
#include <filesystem>

#include "Core.h"

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

        // future use asset resource & metadata to load shader from file
        // like ShaderLibrary->Load(name, path);
        // using Ref to manage ShaderLibrary & Shader
        static Shader* Create(const std::filesystem::path& filepath);
        virtual const std::string& GetName() const = 0;
    };
    
    // finnaly handled by ASSET system
    class ShaderLibrary {
        public:
            ~ShaderLibrary() = default;
            void Add(const Ref<Shader> &shader);
            Ref<Shader> Load(const std::string &name, const std::filesystem::path &filepath);
            Ref<Shader> Load(const std::filesystem::path &filepath);

            const Ref<Shader> Get(const std::string &name) const;
            std::unordered_map<std::string, Ref<Shader>>& GetShaders() { return m_shaders; }
            const std::unordered_map<std::string, Ref<Shader>>& GetShaders() const { return m_shaders; }

        private:
            std::unordered_map<std::string, Ref<Shader>> m_shaders;
    };
}