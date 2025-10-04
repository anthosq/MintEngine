#pragma once

// TODO: 需要修正路径问题
#include "../../render/window_system.h"
#include "../../render/graphics_context.h"
#include "../../render/gl_common.h"

#include <string>
#include <array>

namespace Mint {

    class WindowsWindow : public Window {
        public:
            WindowsWindow(const WindowCreateInfo& create_info);
            virtual ~WindowsWindow();

            void OnUpdate() override;

            unsigned int GetWidth() const override { return m_data.width; }
            unsigned int GetHeight() const override { return m_data.height; }

            // Window attributes
            void SetEventCallback(const EventCallbackFn& callback) override;
            void SetVsync(bool enabled) override;
            bool IsVsync() const override;
            
            bool getFocusMode() const override;
            void setFocusMode(bool is_focus) override;

            virtual void* GetNativeWindow() const { return m_window; }

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

                GraphicContext* m_context {nullptr};

                GLFWwindow* m_window {nullptr};
                WindowData m_data;

    };
}