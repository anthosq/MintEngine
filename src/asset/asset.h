#pragma once
#include "Core/ref.h"
#include "uuid.h"

namespace Mint {
    using AssetHandle = UUID;
    class Asset : public RefCounter {
    // 临时接口, 后续实现Asset时完善
    // 目前先作为所有资源的基类存在
    public:
        AssetHandle Handle = 0;
        Asset() = default;
        virtual ~Asset() = default;
    };

    std::unordered_map<AssetHandle, Ref<Asset>> s_loaded_assets;
    // 临时的AssetManager 作为单例管理所有资源
    class AssetManager {
    public:
        template<typename T>
        static Ref<T> GetAsset(AssetHandle handle) {
            // 后续Internal的工作会转接给各自的manager
            Ref<Asset> asset = GetAssetInternal(handle);
            return asset ? asset.As<T>() : nullptr;
        }

        static AssetHandle AddMemoryOnlyAsset(const Ref<Asset>& asset) {
            if (asset->Handle == 0) {
                asset->Handle = UUID();
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