#pragma once

#include "../../render/window_system.h"
#include <GLFW/glfw3.h>
#include <string>
#include <array>


namespace Mint {

    class WindowsWindow : public Window {
        public:
            WindowsWindow(const WindowCreateInfo& create_info);
            virtual ~WindowsWindow();

            void OnUpdate() override;

            std::array<unsigned int, 2> GetWindowSize() const override;

            // Window attributes
            void SetEventCallback(const EventCallbackFn& callback) override;
            void SetVsync(bool enabled) override;
            bool IsVsync() const override;
            
            bool getFocusMode() const override;
            void setFocusMode(bool is_focus) override;

            virtual GLFWwindow* GetWindow() const { return m_window; }

            private:
                virtual void Init(const WindowCreateInfo& create_info);
                virtual void Shutdown();
            
            private:
                struct WindowData
                {
                    // TODO: 使用自定义字符串类替换std::string
                    std::string title;
                    unsigned int width {0}, height {0};
                    bool v_sync {true};
                    bool m_is_focus {false};
                    EventCallbackFn event_callback;
                };
                
                GLFWwindow* m_window {nullptr};
                WindowData m_data;

    };
}