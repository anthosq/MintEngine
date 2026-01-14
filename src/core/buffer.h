#pragma once
#include <cstdint>

namespace Mint {
    using byte = unsigned char;

    // TODO: 考虑改进这部分内容, 引入OwnsMemory标志位?
    // 然后实现Rule of Five?

    struct Buffer {
        byte* Data;
        uint32_t Size;

        Buffer() : Data(nullptr), Size(0) {}

        Buffer(byte* data, uint32_t size) : Data(data), Size(size) {}

        static Buffer Copy(const Buffer& other) {
            Buffer buffer;
            buffer.Allocate(other.Size);
            memcpy(buffer.Data, other.Data, other.Size);
            return buffer;
        }

        static Buffer Copy(void* data, uint32_t size) {
            Buffer buffer;
            buffer.Allocate(size);
            memcpy(buffer.Data, data, size);
            return buffer;
        }

        void Allocate(uint32_t size) {
            delete[] (byte*)Data;
            Data = nullptr;

            if (size == 0)
                return;

            Data = new byte[size];
            Size = size;
        }

        void Release() {
            delete[] (byte*)Data;
            Data = nullptr;
            Size = 0;
        }

        void ZeroInitialize() {
            if (Data)
                memset(Data, 0, Size);
        }

        void Write(byte* data, uint32_t size, uint32_t offset = 0) {
            // ASSERT(offset + size <= Size, "Buffer overflow!");
            memcpy(Data + offset, data, size);
        }

        template <typename T>
        T& Read(uint64_t offset = 0) {
            return *(T*)(Data + offset);
        }

        template <typename T>
        const T& Read(uint64_t offset = 0) const {
            return *(T*)(Data + offset);
        }

        // read bytes


        
        operator bool() const {
            return Data;
        }

        byte& operator[](int index) {
            return ((byte*)Data)[index];
        }

        byte operator[](int index) const {
            return ((byte*)Data)[index];
        }

        template<typename T>
        T* As() {
            return (T*)Data;
        }

        inline uint32_t GetSize() const { return Size; }
    };
}