#pragma once
#include <cstdint>
#include "core/ref.h"


namespace Mint {

    class UniformBuffer : public RefCounter {
    public:
        virtual ~UniformBuffer();
        virtual void SetData(const void* data, uint32_t size, uint32_t offset) = 0;
        virtual void RenderThread_SetData(const void* data, uint32_t size, uint32_t offset) = 0;

        static Ref<UniformBuffer> Create(uint32_t size);

    };
}
