#pragma once
#include "Core/ref.h"
#include "uuid.h"

namespace Mint {
    using AssetHandle = UUID;
    enum class AssetFlag : uint16_t {
        None = 0,
        Missing = 1 << 0,
        Invalid = 1 << 1
    };


    class Asset : public RefCounter {
    // 临时接口, 后续实现Asset时完善
    // 目前先作为所有资源的基类存在
    public:
        AssetHandle Handle = 0;
        Asset() = default;
        virtual ~Asset() = default;
        uint16_t Flags = (uint16_t)AssetFlag::None;

        virtual bool operator==(const Asset& other) const {
            return Handle == other.Handle;
        }

        virtual bool operator!=(const Asset& other) const {
            return !(*this == other);
        }

    private:
        friend class AssimpMeshImporter;

        bool IsValid() const { return ((Flags & (uint16_t)AssetFlag::Missing) | (Flags & (uint16_t)AssetFlag::Invalid)) == 0; }
        bool IsFlagSet(AssetFlag flag) const { return (Flags & (uint16_t)flag); }
        void SetFlag(AssetFlag flag, bool value = true) {
            if (value) {
                Flags |= (uint16_t)flag;
            } else {
                Flags &= ~(uint16_t)flag;
            }
        }

    };

    // 临时的AssetManager 作为单例管理所有资源
    class AssetManager {
    public:
        template<typename T>
        static Ref<T> GetAsset(AssetHandle handle) {
            // 后续Internal的工作会转接给各自的manager
            Ref<Asset> asset = GetAssetInternal(handle);
            return asset ? asset.As<T>() : nullptr;
        }

        static AssetHandle AddMemoryOnlyAsset(Ref<Asset>& asset) {
            if (asset->Handle == 0) {
                asset->Handle = AssetHandle();
            }
            s_loaded_assets[asset->Handle] = asset;
            return asset->Handle;
        }

        static bool IsAssetValid(AssetHandle handle) {
            return s_loaded_assets.find(handle) != s_loaded_assets.end();
        }

    private:
        static Ref<Asset> GetAssetInternal(AssetHandle handle) {
            auto it = s_loaded_assets.find(handle);
            if (it != s_loaded_assets.end()) {
                return it->second;
            }
            return nullptr;
        }

        static std::unordered_map<AssetHandle, Ref<Asset>> s_loaded_assets;
    };
}