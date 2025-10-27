#pragma once
// later using custom memory allocator
#include <memory>

#include <atomic>
#include <cstddef>
#include <type_traits>

/*  
    basically intrusive reference counting smart pointer implementation.
    prepared for asset management system in the future.
*/



// WIP.
namespace Mint {
    // Reference counting smart pointer
    class RefCounter {
        public:
            virtual ~RefCounter() = default;
            
            // 应该防止拷贝导致引用计数混乱？
            // RefCounter(const RefCounter &) = delete;
            // RefCounter &operator=(const RefCounter &) = delete;

            void IncRefCount()
            {
                m_ref_count.fetch_add(1, std::memory_order_relaxed);
            }

            void DecRefCount() {
                // not sure whether to handle the delete in RefCounter
                // perhaps better in Ref class
                // if (m_ref_count.fetch_sub(1, std::memory_order_acq_rel) == 1) {
                //     delete this;
                // }
                m_ref_count.fetch_sub(1, std::memory_order_acq_rel);
            }

            uint32_t GetRefCount() const {
                // 使用强内存序, 因为Ref类的删除应该由Ref类负责？
                return m_ref_count.load(std::memory_order_acquire);
            }
    

        private:
            mutable std::atomic<uint32_t> m_ref_count{0};
    };

    namespace RefUtils {

    }

    template<typename T>
    class Ref {
        public:
            Ref() : m_instance(nullptr) {}

            Ref(std::nullptr_t) : m_instance(nullptr) {}

            Ref(T* instance) : m_instance(instance) {
                static_assert(std::is_base_of<RefCounter, T>::value, "Ref<T>: T must inherit from RefCounter");
                
                
            }
        private:
            mutable T* m_instance = nullptr;
    };
}