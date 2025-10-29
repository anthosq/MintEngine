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
            RefCounter() = default;
            virtual ~RefCounter() = default;
            
            RefCounter(const RefCounter &) = delete;
            RefCounter &operator=(const RefCounter &) = delete;

            void IncRefCount() const {
                // m_ref_count.fetch_add(1, std::memory_order_relaxed);
                ++m_ref_count;
            }

            void DecRefCount() const {
                // m_ref_count.fetch_sub(1, std::memory_order_acq_rel);
                --m_ref_count;
            }

            uint32_t GetRefCount() const {
                return m_ref_count.load();
            }
    

        private:
            mutable std::atomic<uint32_t> m_ref_count{0};
    };

    namespace RefUtils {
        void AddToLiveRef(void* instance);
        void RemoveFromLiveRef(void* instance);
        bool IsAlive(void* instance);
        void DumpLiveRefs();
    }

    template<typename T>
    class Ref {

        /*  1. 从裸指针构造
            2. 拷贝构造
            3. 移动构造
            4. 析构
            5. 赋值
            6. 重载-> 和 * 运算符
            7. 提供获取裸指针的方法
            8. 提供静态方法从另一个Ref创建但不增加引用计数的Ref
            9. 类型转换支持
            10. Create方法                                 */

        public:
            Ref() : m_instance(nullptr) {}

            Ref(std::nullptr_t) : m_instance(nullptr) {}

            Ref(T* instance) : m_instance(instance) {
                static_assert(std::is_base_of<RefCounter, T>::value, "Ref<T>: T must inherit from RefCounter");
                IncRef();
            }

            template<typename T2>
            Ref(const Ref<T2>& other) {
                m_instance = (T*)other.m_instance;
                IncRef();
            }

            template<typename T2>
            Ref(Ref<T2>&& other) {
                // 移动, 转移所有权, 不增加计数
                m_instance = (T*)other.m_instance;
                other.m_instance = nullptr;
            }

            static Ref<T> CopyWithoutInc(const Ref<T>& other) {
                Ref<T> ref = nullptr;
                ref.m_instance = other.m_instance;
                return ref;
            }

            ~Ref() {
                DecRef();
            }

            Ref(const Ref<T>& other) {
                m_instance = other.m_instance;
                IncRef();
            }

            Ref& operator=(std::nullptr_t) {
                DecRef();
                m_instance = nullptr;
                return *this;
            }

            Ref& operator=(const Ref<T>& other) {
                if (this == &other) {
                    return *this;
                }
                other.IncRef();
                DecRef();

                m_instance = other.m_instance;
                return *this;
            }

            template<typename T2>
            Ref& operator=(const Ref<T2>& other) {
                other.IncRef();
                DecRef();

                m_instance = other.m_instance;
                return *this;
            }

            template<typename T2>
            Ref& operator=(Ref<T2>&& other) {
                DecRef();

                m_instance = other.m_instance;
                other.m_instance = nullptr;
                return *this;
            }

            T* Raw() { return m_instance; }
            const T* Raw() const { return m_instance; }

            void Reset(T* instance = nullptr) {
                DecRef();
                m_instance = instance;
            }

            bool Equals(const Ref<T>& other) const {
                return m_instance != nullptr &&
                       other.m_instance != nullptr &&
                       m_instance == other.m_instance;
            }

            template<typename T2>
            Ref<T2> As() {
                return Ref<T2>(*this);
            }

            T* operator->() { return m_instance; }
            const T* operator->() const { return m_instance; }

            T& operator*() { return *m_instance; }
            const T& operator*() const { return *m_instance; }

            operator bool() { return m_instance != nullptr; }
            operator bool() const { return m_instance != nullptr; }

            bool operator==(const Ref<T>& other) const {
                return m_instance == other.m_instance;
            }

            bool operator!=(const Ref<T>& other) const {
                return m_instance != other.m_instance;
            }

            template<typename... Args>
            static Ref<T> Create(Args&&... args) {
                // 使用完美转发构造T的实例
                return Ref<T>(new T(std::forward<Args>(args)...));
            }
        private:
            void IncRef() const {
                if (m_instance) {
                    m_instance->IncRefCount();
                    RefUtils::AddToLiveRef((void*)m_instance);
                }
            }

            void DecRef() const {
                if (m_instance) {
                    // 不确定这部分是不是该放在RefCounter里处理？
                    // m_instance为mutable，可以在const函数中修改
                    m_instance->DecRefCount();
                    // 会存在竞态吗？
                    // NEED CHECK
                    if (m_instance->GetRefCount() == 0) {
                        RefUtils::RemoveFromLiveRef((void*)m_instance);
                        delete std::exchange(m_instance, nullptr);
                    }
                }
            }


        private:
            template<typename U>
            friend class Ref;
            mutable T* m_instance;
    };

    template <typename T>
    class WeakRef {
        public:
            WeakRef() = default;
            WeakRef(Ref<T> ref) { m_instance = ref.Raw(); }
            WeakRef(T* instance) { m_instance = instance; }

            T* operator->() { return m_instance; }
            const T* operator->() const { return m_instance; }

            T& operator*() { return *m_instance; }
            const T& operator*() const { return *m_instance; }

            bool IsValid() const { return m_instance ? RefUtils::IsAlive(m_instance) : false; }
            operator bool() const { return IsValid(); }

            template<typename T2>
            WeakRef<T2> As() {
                return WeakRef<T2>(dynamic_cast<T2*>(m_instance));
            }

        private:
            T* m_instance = nullptr;
    };
}