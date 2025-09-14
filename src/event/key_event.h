#pragma once

#include "event_system.h"
#include <functional>
#include <sstream>

namespace Mint {
    class KeyEvent : public Event {
        public:
            inline int GetKeyCode() const { return m_KeyCode; }

            EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)

        protected:
            KeyEvent(int keycode)
                : m_KeyCode(keycode) {}

        private:
            int m_KeyCode;
    };

    class KeyPressedEvent : public KeyEvent {
        public:
            KeyPressedEvent(int keycode, int repeatCount)
                : KeyEvent(keycode), m_RepeatCount(repeatCount) {}

            inline int GetRepeatCount() const { return m_RepeatCount; }

            std::string ToString() const override {
                std::stringstream ss;
                ss << "KeyPressedEvent: " << GetKeyCode() << " (" << m_RepeatCount << " repeats)";
                return ss.str();
            }

            EVENT_CLASS_TYPE(KeyPressed)

        private:
            int m_RepeatCount;
    };

    class KeyReleasedEvent : public KeyEvent {
        public:
            KeyReleasedEvent(int keycode)
                : KeyEvent(keycode) {}

            std::string ToString() const override {
                std::stringstream ss;
                ss << "KeyReleasedEvent: " << GetKeyCode();
                return ss.str();
            }

            EVENT_CLASS_TYPE(KeyReleased)
    };
}
