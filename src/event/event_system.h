#pragma once

#include <functional>
// TODO: 考虑移除对 <string> 的依赖, 使用自定义的字符串类
#include <string>


namespace Mint {


    enum class EventType {
        None = 0,
        WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
        AppTick, AppUpdate, AppRender,
        KeyPressed, KeyReleased, KeyTyped,
        MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled, MouseScrolling
    };

    // Bitfield for event categories
    enum EventCategory {
        None = 0,
        EventCategoryApplication    = 1 << 0,
        EventCategoryInput          = 1 << 1,
        EventCategoryKeyboard       = 1 << 2,
        EventCategoryMouse          = 1 << 3,
        EventCategoryMouseButton    = 1 << 4
    };

    // TODO: 考虑使用 constexpr 函数替代宏？ 若不代替, 则移动到统一的宏定义头文件中
#define EVENT_CLASS_CATEGORY(category) \
    virtual int GetCategoryFlags() const override { return category; }

#define EVENT_CLASS_TYPE(type) \
    static EventType GetStaticType() { return EventType::##type; } \
    virtual EventType GetEventType() const override { return GetStaticType(); } \
    virtual const char* GetName() const override { return #type; }


// 考虑 using Mint::string = std::string; 便于后期实现自定义string时, 统一替换
    class Event {
        friend class EventDispatcher;
        public:
            virtual EventType GetEventType() const = 0;
            virtual const char* GetName() const = 0;
            virtual int GetCategoryFlags() const = 0;
            virtual std::string  ToString() const { return GetName(); }

            bool IsInCategory(EventCategory category) {
                return GetCategoryFlags() & category;
            }

        protected:
            bool m_Handled = false; 

    };

    class EventDispatcher {
        public:
        // template<typename T>
        // using EventCallbackFn = std::function<bool(T&)>;
            EventDispatcher(Event& event)
                : m_event(event) {}

            // F will be deduced by the compiler
            template<typename T, typename F>
            bool Dispatch(const F& func) {
                if (m_event.GetEventType() == T::GetStaticType()) {
                    m_event.m_Handled = func(static_cast<T&>(m_event));
                    return true;
                }
                return false;
            }
        private:
            Event& m_event;
    };

    inline std::ostream& operator<<(std::ostream& os, const Event& e) {
        return os << e.ToString();
    }

}