#pragma once

#include "render/material.h"
#include "core/buffer.h"

namespace Mint {
    class OpenGLMaterial : public Material {
    public:
        OpenGLMaterial(const Ref<Shader>& shader, const std::string& name);
        OpenGLMaterial(Ref<Material>& other, const std::string& name);
        virtual ~OpenGLMaterial() override;

        virtual void Invalidate() override;
        virtual void OnShaderReload() override;

        virtual void SetFlags(uint32_t flags) override;
        virtual void SetFlag(MaterialFlag flag, bool value = true) override;

        virtual void Set(const std::string& name, float value) override;
        virtual void Set(const std::string& name, int value) override;
        virtual void Set(const std::string& name, uint32_t value) override;
        virtual void Set(const std::string& name, bool value) override;

        virtual void Set(const std::string& name, const glm::vec2& value) override;
        virtual void Set(const std::string& name, const glm::vec3& value) override;
        virtual void Set(const std::string& name, const glm::vec4& value) override;

        virtual void Set(const std::string& name, const glm::ivec2& value) override;
        virtual void Set(const std::string& name, const glm::ivec3& value) override;
        virtual void Set(const std::string& name, const glm::ivec4& value) override;

        virtual void Set(const std::string& name, const glm::mat3& value) override;
        virtual void Set(const std::string& name, const glm::mat4& value) override;

        virtual void Set(const std::string& name, const Ref<Texture2D>& texture) override;
        virtual void Set(const std::string& name, const Ref<Texture2D>& texture, uint32_t arrayIndex) override;
        virtual void Set(const std::string& name, const Ref<TextureCube>& texture) override;


        virtual uint32_t GetFlags() const override;
        virtual bool GetFlag(MaterialFlag flag) const override;

        virtual Ref<Shader> GetShader() const override;
        virtual const std::string& GetName() const override;

        virtual float& GetFloat(const std::string& name) const override;
        virtual int& GetInt(const std::string& name) const override;
        virtual uint32_t& GetUInt(const std::string& name) const override;
        virtual bool& GetBool(const std::string& name) const override;

        virtual glm::vec2& GetVec2(const std::string& name) const override;
        virtual glm::vec3& GetVec3(const std::string& name) const override;
        virtual glm::vec4& GetVec4(const std::string& name) const override;

        virtual glm::ivec2& GetIVec2(const std::string& name) const override;
        virtual glm::ivec3& GetIVec3(const std::string& name) const override;
        virtual glm::ivec4& GetIVec4(const std::string& name) const override;

        virtual glm::mat3& GetMat3(const std::string& name) const override;
        virtual glm::mat4& GetMat4(const std::string& name) const override;

        virtual Ref<Texture2D>& GetTexture2D(const std::string& name) const override;
        virtual Ref<TextureCube>& GetTextureCube(const std::string& name) const override;


        const ShaderUniform* FindUniformDeclaration(const std::string& name) const;

        // 使用template
        template <typename T>
        void Set(const std::string& name, const T& value) {
            const ShaderUniform* decl = FindUniformDeclaration(name);
            // 静态断言, 不支持的类型会在编译时报错
            // static_assert(decl, "uniform not found");
            if (decl) {
                SetUniformValue(decl, value);
            }
            Buffer &buffer = m_UniformBufferData;
            buffer.Write((byte*)&value, decl->GetSize(), decl->GetOffset());
        }

        template <typename T>
        T& Get(const std::string name) {
            const ShaderUniform* decl = FindUniformDeclaration(name);
            // 静态断言, 不支持的类型会在编译时报错
            // static_assert(decl, "uniform not found");
            Buffer &buffer = m_UniformBufferData;
            return buffer.Read<T>(decl->GetOffset());
        }

    private:
        uint32_t m_MaterialFlags = 0;
        Buffer m_UniformBufferData;
    };
}