#pragma once

#include <vector>
#include <string>
#include "render/interface/opengl/gl_common.h"

namespace Mint {
    //temporary

    enum class ShaderDataType {
        None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
    };

    static GLenum GetOpenGLDataType(ShaderDataType type) {
        switch (type) {
            case ShaderDataType::Float:   return GL_FLOAT;
            case ShaderDataType::Float2:  return GL_FLOAT;
            case ShaderDataType::Float3:  return GL_FLOAT;
            case ShaderDataType::Float4:  return GL_FLOAT;
            case ShaderDataType::Mat3:    return GL_FLOAT;
            case ShaderDataType::Mat4:    return GL_FLOAT;
            case ShaderDataType::Int:     return GL_INT;
            case ShaderDataType::Int2:    return GL_INT;
            case ShaderDataType::Int3:    return GL_INT;
            case ShaderDataType::Int4:    return GL_INT;
            case ShaderDataType::Bool:    return GL_BOOL;
        }
        // 后续采用Assert替代LOG_ERROR
        return 0;
    }

    static uint32_t ShaderDataTypeSize(ShaderDataType type) {
        switch (type) {
            case ShaderDataType::Float:   return 4;
            case ShaderDataType::Float2:  return 4 * 2;
            case ShaderDataType::Float3:  return 4 * 3;
            case ShaderDataType::Float4:  return 4 * 4;
            case ShaderDataType::Mat3:    return 4 * 3 * 3;
            case ShaderDataType::Mat4:    return 4 * 4 * 4;
            case ShaderDataType::Int:     return 4;
            case ShaderDataType::Int2:    return 4 * 2;
            case ShaderDataType::Int3:    return 4 * 3;
            case ShaderDataType::Int4:    return 4 * 4;
            case ShaderDataType::Bool:    return 1;
        }
        // 后续采用Assert替代LOG_ERROR
        return 0;
    }

    class BufferElement {
        public:
            std::string     name;
            ShaderDataType  type;
            uint32_t        size;
            uint32_t        offset;
            bool            normalized;

            BufferElement() = default;
            BufferElement(ShaderDataType type, const std::string& name, bool normalized = false)
                : name(name), type(type), size(ShaderDataTypeSize(type)), offset(0), normalized(normalized) {
                }

            uint32_t GetComponentCount() const {
                switch (type) {
                    case ShaderDataType::Float:   return 1;
                    case ShaderDataType::Float2:  return 2;
                    case ShaderDataType::Float3:  return 3;
                    case ShaderDataType::Float4:  return 4;
                    case ShaderDataType::Mat3:    return 3 * 3;
                    case ShaderDataType::Mat4:    return 4 * 4;
                    case ShaderDataType::Int:     return 1;
                    case ShaderDataType::Int2:    return 2;
                    case ShaderDataType::Int3:    return 3;
                    case ShaderDataType::Int4:    return 4;
                    case ShaderDataType::Bool:    return 1;
                }
                return 0;
            }
    };


    class BufferLayout {
        public:
            BufferLayout() = default;
            BufferLayout(const std::initializer_list<BufferElement>& element) 
            : m_elements(element) {
                // Calculate offset and stride
                uint32_t offset = 0;
                m_stride = 0;
                for (auto& elem : m_elements) {
                    elem.offset = offset;
                    offset += elem.size;
                    m_stride += elem.size;
                }
            }

            ~BufferLayout() = default;

            inline const std::vector<BufferElement>& GetElements() const { return m_elements; }
            inline uint32_t GetStride() const { return m_stride; }
            
            std::vector<BufferElement>::iterator begin() { return m_elements.begin(); }
            std::vector<BufferElement>::iterator end() { return m_elements.end(); }
            std::vector<BufferElement>::const_iterator begin() const { return m_elements.begin(); }
            std::vector<BufferElement>::const_iterator end() const { return m_elements.end(); }

        private:
            std::vector<BufferElement> m_elements;
            uint32_t m_stride = 0;
    };

    // abstract class
    class VertexBuffer {
        public:
            virtual ~VertexBuffer() = default;

            virtual void Bind() const = 0;
            virtual void Unbind() const = 0;

            virtual const BufferLayout& GetLayout() const = 0;
            virtual void SetLayout(const BufferLayout& layout) = 0;

            virtual void SetData(const void* data, uint32_t size) = 0;

            static VertexBuffer* Create(uint32_t size);
            static VertexBuffer* Create(float* vertices, uint32_t size);
    };


    class IndexBuffer {
        public:
            virtual ~IndexBuffer() = default;

            virtual void Bind() const = 0;
            virtual void Unbind() const = 0;
            virtual uint32_t GetCount() const = 0;

            static IndexBuffer* Create(uint32_t* indices, uint32_t count);
    };
    
}