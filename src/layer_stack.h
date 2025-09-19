#pragma once

#include "layer.h"
#include <vector>
#include <algorithm>
// TODO: 使用Mint的vector, 对std::vector进行封装

namespace Mint {
    class LayerStack {
        public:
            LayerStack();
            ~LayerStack();

            void PushLayer(Layer* layer);
            void PushOverlay(Layer* overlay);
            void PopLayer(Layer* layer);
            void PopOverlay(Layer* overlay);

            std::vector<Layer*>::iterator begin() {
                return m_layers.begin();
            }

            std::vector<Layer*>::iterator end() {
                return m_layers.end();
            }

        private:
            std::vector<Layer*> m_layers;
            std::vector<Layer*>::iterator m_layer_insert;
    };
}