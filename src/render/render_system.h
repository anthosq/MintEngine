#pragma once

namespace Mint {
    enum class RenderAPI {
        None = 0,
        OpenGL = 1,
    };

    class RenderSystem {
    public:
        // temporary
        static RenderAPI GetAPI() { return s_api; }
        static void SetAPI(RenderAPI api = RenderAPI::OpenGL) { s_api = api; }

        static RenderAPI s_api;
    };
}