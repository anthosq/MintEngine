#pragma once

#include "event/event_system.h"
#include "core/time_step.h"

namespace Mint {
    class Layer {
        public:
            Layer(const std::string& name ="Layer");
            virtual ~Layer();

            virtual void OnAttach() {}
            virtual void OnDetach() {}
            virtual void OnUpdate(TimeStep delta_time) {}
            virtual void OnEvent(Event& event) {}

            // Not sure if we need this function
            virtual void OnImGuiRender() {}

            inline const std::string& GetName() const { return m_debug_name; }

        protected:
            std::string m_debug_name;
    };
}