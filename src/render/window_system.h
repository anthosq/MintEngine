#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <functional>
#include "event/event_system.h"

namespace Mint {
    struct WindowCreateInfo {
        unsigned int width = 1980;
        unsigned int height = 1020;
        const char* title = "Mint Engine";

        bool is_fullscreen = false;
        
    };

    class Window {
        public:
            using EventCallbackFn = std::function<void(Event&)>;
            // WindowSystem() = default;
            virtual ~Window() = default;
            virtual void OnUpdate() = 0; // PollEvents

            // virtual std::array<unsigned int, 2> GetWindowSize() const = 0;

            virtual unsigned int GetWidth() const = 0;
            virtual unsigned int GetHeight() const = 0;

            // Window attributes
            virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
            /*
            void SetVSync(bool enabled);
            bool IsVSync() const;
            */


            virtual void SetVsync(bool enabled) = 0;
            virtual bool IsVsync() const = 0;
            virtual bool getFocusMode() const = 0;
            virtual void setFocusMode(bool is_focus) = 0;
            virtual void* GetNativeWindow() const = 0;

            static Window* Create(const WindowCreateInfo& create_info);
    };
}
