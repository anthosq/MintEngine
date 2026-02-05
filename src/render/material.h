#pragma once

#include "Core/ref.h"
#include "render/shader.h"
#include "render/texture.h"
#include <string>

namespace Mint {

    // Material flags用于指示DepthTest, Blend等状态
    enum class MaterialFlag {
        None = 1 << 0,
        DepthTest = 1 << 1,
        Blend = 1 << 2,
        TwoSided = 1 << 3,
        DisableShadowCast = 1 << 4,
    };


    class Material : public RefCounter {
    public:
        static Ref<Material> Create(const Ref<Shader>& shader, const std::string& name = "");
        static Ref<Material> Copy(const Ref<Material>& other, const std::string& name = "");

        virtual ~Material() = default;

        virtual void Invalidate() = 0;
        virtual void OnShaderReload() = 0;


        // Set Method
        virtual void SetFlags(uint32_t flags) = 0;
        virtual void SetFlag(MaterialFlag flag, bool value = true) = 0;



        // Scalar
        virtual void Set(const std::string& name, float value) = 0;
        virtual void Set(const std::string& name, int value) = 0;
        virtual void Set(const std::string& name, uint32_t value) = 0;
        virtual void Set(const std::string& name, bool value) = 0;

        // Vector
        virtual void Set(const std::string& name, const glm::vec2& value) = 0;
        virtual void Set(const std::string& name, const glm::vec3& value) = 0;
        virtual void Set(const std::string& name, const glm::vec4& value) = 0;

        virtual void Set(const std::string& name, const glm::ivec2& value) = 0;
        virtual void Set(const std::string& name, const glm::ivec3& value) = 0;
        virtual void Set(const std::string& name, const glm::ivec4& value) = 0;

        // Matrix
        virtual void Set(const std::string& name, const glm::mat3& value) = 0;
        virtual void Set(const std::string& name, const glm::mat4& value) = 0;

        // Texture
        // Texture类本身是虚基类
        virtual void Set(const std::string& name, const Ref<Texture2D>& texture) = 0;
        virtual void Set(const std::string& name, const Ref<Texture2D>& texture, uint32_t arrayIndex) = 0;
        virtual void Set(const std::string& name, const Ref<TextureCube>& texture) = 0;
    
        // Get Methods
        virtual uint32_t GetFlags() const = 0;
        virtual bool GetFlag(MaterialFlag flag) const = 0;

        virtual Ref<Shader> GetShader() const = 0;
        virtual const std::string& GetName() const = 0;

        // 我不认为这里应该传递引用, 如果使用Setter的设计模式
        // 但是考虑到配合ImGui, 这里先设计为传递引用, 这一点之后需要注意
        // Scalar
        virtual float& GetFloat(const std::string& name) = 0;
        virtual int& GetInt(const std::string& name) = 0;
        virtual uint32_t& GetUInt(const std::string& name) = 0;
        virtual bool& GetBool(const std::string& name) = 0;

        // Vector
        virtual glm::vec2& GetVec2(const std::string& name) = 0;
        virtual glm::vec3& GetVec3(const std::string& name) = 0;
        virtual glm::vec4& GetVec4(const std::string& name) = 0;

        virtual glm::ivec2& GetIVec2(const std::string& name) = 0;
        virtual glm::ivec3& GetIVec3(const std::string& name) = 0;
        virtual glm::ivec4& GetIVec4(const std::string& name) = 0;

        // Matrix
        virtual glm::mat3& GetMat3(const std::string& name) = 0;
        virtual glm::mat4& GetMat4(const std::string& name) = 0;

        // Texture
        virtual Ref<Texture2D> GetTexture2D(const std::string& name) = 0;
        virtual Ref<TextureCube> GetTextureCube(const std::string& name) = 0;

    };
}