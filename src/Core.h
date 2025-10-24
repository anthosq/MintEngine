#pragma once
#include <memory>

//temporary macros
#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

// basic mappers

namespace Mint {

    // 后续拆分为Ref类与Scope类
    // 进行封装

    // 独占式指针使用unique_ptr管理, 确保所有权
    template<typename T>
    using Scope = std::unique_ptr<T>;

    // Asset使用shared_ptr管理
    template<typename T>
    using Ref = std::shared_ptr<T>;


}