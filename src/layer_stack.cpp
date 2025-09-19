#include "layer_stack.h"


namespace Mint {
    LayerStack::LayerStack() {
        m_layer_insert = m_layers.begin();
    }

    LayerStack::~LayerStack() {
        for (Layer* layer : m_layers) {
            layer->OnDetach();
            delete layer;
        }
    }

    void LayerStack::PushLayer(Layer* layer) {
    m_layer_insert = m_layers.emplace(m_layer_insert, layer);
    }

    // 确保Overlay在最上层
    void LayerStack::PushOverlay(Layer* overlay) {
        m_layers.emplace_back(overlay);
    }

    void LayerStack::PopLayer(Layer* layer) {
        auto it = std::find(m_layers.begin(), m_layers.end(), layer);
        if (it != m_layers.end()) {
            layer->OnDetach();
            m_layers.erase(it);
            m_layer_insert--;
        }
    }

    void LayerStack::PopOverlay(Layer* Overlay) {
        auto it = std::find(m_layers.begin(), m_layers.end(), Overlay);
        if (it != m_layers.end()) {
            Overlay->OnDetach();
            m_layers.erase(it);
        }
    }
}