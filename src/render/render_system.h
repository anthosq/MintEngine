#pragma once
#include "render_command.h"


namespace Mint {

    class RenderSystem {
    public:
        static void BeginScene();

        static void EndScene();

        static void Submit(const std::shared_ptr<VertexArray>& vertex_array);


        static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
    };
}