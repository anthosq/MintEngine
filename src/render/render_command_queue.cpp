#include "render/render_command_queue.h"
#include <cstring>
#include <memory>
#include <cstddef>

namespace Mint {
    RenderCommandQueue::RenderCommandQueue()
    {
        // initialize Command Buffer
        m_CommandBuffer = new uint8_t[1024 * 1024 * 10]; // 10MB command buffer
        m_CommandBufferPtr = m_CommandBuffer;
        memset(m_CommandBuffer, 0, 1024 * 1024 * 10);
    }

    RenderCommandQueue::~RenderCommandQueue()
    {
        // destroy Command Buffer
        delete[] m_CommandBuffer;
    }
    void* RenderCommandQueue::Allocate(RenderCommandFn func, uint32_t size)
    {
        // // attempt to align the command buffer pointer
        // constexpr size_t alignment = alignof(std::max_align_t);
        // size_t space = m_CommandBuffer + 1024 * 1024 * 10 - m_CommandBufferPtr;
        // uint8_t* alignedPtr = (uint8_t*)std::align(alignment, sizeof(RenderCommandFn) + sizeof(uint32_t) + size, (void*&)m_CommandBufferPtr, space);

        // if (!alignedPtr) {
        //     // TODO: Handle out-of-memory or alignment failure
        //     return nullptr;
        // }

        // uint8_t* cmdPtr = alignedPtr;

        // // Store the function pointer
        // *(RenderCommandFn*)cmdPtr = func;
        // cmdPtr += sizeof(RenderCommandFn);

        // // Store the size
        // *(uint32_t*)cmdPtr = size;
        // cmdPtr += sizeof(uint32_t);

        // // Store the command data
        // void* dataPtr = (void*)cmdPtr;
        // m_CommandBufferPtr = cmdPtr + size;

        // m_CommandCount++;
        // return dataPtr;
        
		*(RenderCommandFn*)m_CommandBufferPtr = func;
		m_CommandBufferPtr += sizeof(RenderCommandFn);

		*(uint32_t*)m_CommandBufferPtr = size;
		m_CommandBufferPtr += sizeof(uint32_t);

		void* memory = m_CommandBufferPtr;
		m_CommandBufferPtr += size;

		m_CommandCount++;
		return memory;
    }

    void RenderCommandQueue::Execute()
    {
        uint8_t* cmdPtr = m_CommandBuffer;

        for (uint32_t i = 0; i < m_CommandCount; i++)
        {
            RenderCommandFn func = *(RenderCommandFn*)cmdPtr;
            cmdPtr += sizeof(RenderCommandFn);

            uint32_t size = *(uint32_t *)cmdPtr;
            cmdPtr += sizeof(uint32_t);
            func((void *)cmdPtr);
            cmdPtr += size;
        }

        // Reset the command buffer
        m_CommandBufferPtr = m_CommandBuffer;
        m_CommandCount = 0;
	}

}