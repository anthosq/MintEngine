#pragma once

#include <string>
#include <cstdint>
#include <memory>
#include <glm/glm.hpp>
#include <filesystem>

#include "core/ref.h"

namespace Mint {
    enum class ShaderUniformType {
        None = 0, Bool, Int, UInt, Float,
        Vec2, Vec3, Vec4, Mat3, Mat4,
        IVec2, IVec3, IVec4
    };

    class ShaderUniform {
        public:
            ShaderUniform() = default;
            ShaderUniform(std::string name, ShaderUniformType type, uint32_t size, uint32_t offset);

            const std::string& GetName() { return m_name; }
            ShaderUniformType GetType() { return m_type; }
            uint32_t GetSize() { return m_size; }
            uint32_t GetOffset() { return m_offset; }

            // 不确定这个是否该在这个类下
            static std::string_view UniformTypeToString(ShaderUniformType type);

            // TODO: 未来需要实现序列化与反序列划

        private:
            std::string m_name;
            ShaderUniformType m_type = ShaderUniformType::None;
            uint32_t m_size;
            uint32_t m_offset;
    };

    // 用于描述Shader中的成组的unform变量, struct等, 如CameraData之类的
    // CPU side buffer abstraction?
    class ShaderBuffer {
        public:
        // TODO: 序列化与反序列化？

            std::string m_name;
            uint32_t m_size;
            std::unordered_map<std::string, ShaderUniform> m_uniforms;
    };



    class Shader : public RefCounter {
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
        static Ref<Shader> Create(const std::filesystem::path& filepath);
        
        virtual const std::string& GetName() const = 0;

        virtual void Reflect() = 0;
        virtual const std::unordered_map<std::string, ShaderUniform>& GetUniforms() const = 0;
    };
    
    // finnaly handled by ASSET system
    class ShaderLibrary : public RefCounter {
        public:
            ~ShaderLibrary() = default;
            void Add(const Ref<Shader> &shader);
            void Load(const std::string &name, const std::filesystem::path &filepath);
            void Load(const std::filesystem::path &filepath);

            const Ref<Shader> Get(const std::string &name) const;
            std::unordered_map<std::string, Ref<Shader>>& GetShaders() { return m_shaders; }
            const std::unordered_map<std::string, Ref<Shader>>& GetShaders() const { return m_shaders; }

        private:
            std::unordered_map<std::string, Ref<Shader>> m_shaders;
    };
}