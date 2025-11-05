#pragma once
#include "Core/ref.h"

namespace Mint {
    class Asset : public RefCounter {
    // 临时接口, 后续实现Asset时完善
    // 目前先作为所有资源的基类存在
    public:
        Asset() = default;
        virtual ~Asset() = default;
    };
}