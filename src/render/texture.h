#pragma once
#include <cstdint>
#include <filesystem>

#include "Core.h"
#include <glm/glm.hpp>

// TODO: 完成Asset系统后, 重构Texture相关类
namespace Mint {
    enum class TextureFormat {
        None = 0,
        RGB8,
        RGBA8,
        SRGB8,
        SRGB_ALPHA8,
    };

    enum class TextureFilter {
        None = 0,
        Linear,
        Nearest,
    };

    enum class TextureWrap {
        None = 0,
        Repeat,
        ClampToEdge,
        MirroredRepeat,
    };    

    enum class TextureType {
        None = 0,
        Texture2D,
        TextureCube,
    };

    struct TextureSpecification
    {
        // Later move to Image.h
        TextureFormat Format = TextureFormat::RGBA8;

        uint32_t Width = 1;
        uint32_t Height = 1;
        TextureFilter MinFilter = TextureFilter::Linear;
        TextureFilter MagFilter = TextureFilter::Linear;
        TextureWrap WrapS = TextureWrap::Repeat;
        TextureWrap WrapT = TextureWrap::Repeat;
        
        // TODO: 后续拓展MipMap相关选项, 以及Storage相关选项
        // bool GenerateMipMaps = true;

    };

    // TODO: 后续接入Asset系统, 继承自RenderableAsset(利用Ref类, 完善Asset系统)
    class Texture {
    public:
        virtual ~Texture() = default;

        virtual void Bind(uint32_t slot) const = 0;

        // TextureFormat命名并不准确, 应该是ImageFormat, 暂时如此
        virtual TextureFormat GetFormat() const = 0;
        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;
        virtual glm::vec2 GetSize() const = 0;

        // TODO: 后续拓展MipMap相关接口

        virtual unsigned int GetRendererID() const = 0;

        virtual TextureType GetType() const = 0;
    };

    // 这些应该在Texture基类中吗？
    // 还是说应该在各个派生类中实现？
    class Texture2D : public Texture {
        public:
        // 对外接口
        static Ref<Texture2D> Create(const TextureSpecification& spec);
        static Ref<Texture2D> Create(const TextureSpecification& spec, const std::filesystem::path& path);

        // 对内部的实现, 由各个渲染器实现
        virtual void CreateFromFile(const TextureSpecification& spec, const std::filesystem::path& path) = 0;

        // virtual void ReplaceFromFile(const TextureSpecification& spec, const std::filesystem::path& path) = 0;


        virtual TextureType GetType() const override { return TextureType::Texture2D; }
        
        virtual bool IsLoaded() const = 0;

        virtual const std::filesystem::path& GetPath() const = 0;
    };


    class TextureCube : public Texture {
    public:
        // temporary
        static Ref<TextureCube> Create(const TextureSpecification& spec, const std::filesystem::path& path);

        virtual TextureType GetType() const override { return TextureType::TextureCube; }

        virtual const std::filesystem::path& GetPath() const = 0;
    };
}