#pragma once
#include <cstdint>
#include "core/ref.h"


namespace Mint {

    class UniformBuffer : public RefCounter {
    public:
        static Ref<UniformBuffer> Create(uint32_t size, uint32_t binding);

        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;



    private:
        uint32_t m_Size;
        uint32_t m_BindingPoint;

    };
}
