#pragma once
#include <cstdint>
#include "core/ref.h"


namespace Mint {

    // Binding的部分不应该安排在这里
    // Binding的部分交给Shader处理, Shader调用SetUniformBuffer来启动Binding

    // Buffer只是CPU端显存的一个抽象, 不应包含Binding的逻辑
    // OpenGLUniformBuffer实现显存分配和数据上传, 提供GetRenderID
    class UniformBuffer : public RefCounter {
    public:
        virtual ~UniformBuffer();
        virtual void SetData(const void* data, uint32_t size, uint32_t offset) = 0;
        virtual void RenderThread_SetData(const void* data, uint32_t size, uint32_t offset) = 0;

        static Ref<UniformBuffer> Create(uint32_t size);
    };
}
