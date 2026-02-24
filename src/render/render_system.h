#pragma once
#include "render/renderer_api.h"
#include "render/render_command_queue.h"
#include "render/camera.h"
#include "render/shader.h"
#include "render/texture.h"
#include "Core.h"
#include "editor/editor_camera.h"

namespace Mint {

    class RenderSystem {
    public:
        typedef void(*RenderCommandFn)(void*);

        // move from render_command
        // In the future: RenderSystem也不持有submit, submit属于RHI层的内容, RenderSystem持有Pipeline

        void Clear();
        // temporary
        void Clear(const glm::vec4& color);

		void DrawIndexed(uint32_t count, bool depthTest = true);

        void DrawArrays(uint32_t mode, uint32_t count, uint32_t first, bool depthTest = true);

        void Init();

        void Shutdown();

        static Ref<ShaderLibrary> GetShaderLibrary();

        template<typename FuncT>
        static void Submit(FuncT&& func) {
            auto renderCmd = [](void* ptr) {
                auto pFunc = (FuncT*)ptr;
                (*pFunc)();

                pFunc->~FuncT();
            };

            // bad design, need to fix
            // need to modify vertexbuffers and so on
            void* storageBuffer = m_renderer->m_commandQueue.Allocate(renderCmd, sizeof(func));
            new (storageBuffer) FuncT(std::forward<FuncT>(func));
        }

        void WaitAndRender();

        void BeginScene(EditorCamera& camera);

        void EndScene();

        void OnWindowResize(uint32_t width, uint32_t height);

        // temporary adding Transform
        void Submit(Ref<Shader>& shader, const Ref<VertexArray>& vertex_array, const glm::mat4& transform = glm::mat4(1.0f), bool depth_test = true);

        void SubmitArrays(Ref<Shader>& shader, const Ref<VertexArray>& vertex_array, uint32_t mode, uint32_t count = 0, uint32_t first = 0, const glm::mat4& transform = glm::mat4(1.0f), bool depth_test = true);

        static RendererAPI::RenderAPIType GetAPI() { return RendererAPI::GetAPIType(); }

        static RenderSystem& Get_Renderer() { return *m_renderer; }

        // temporary Get method for get rendererData
        static Ref<Texture2D> GetWhiteTexture();

        // preparing RenderPass. or maybe render pipeline
		// static void BeginRenderPass(const Ref<RenderPass>& renderPass);
		// static void EndRenderPass();

        // after complete reconstructing rendersystem
        void SubmitMesh() { m_renderer->SubmitMeshImpl(); };
        // not sure, I don't think RendererAPI should based on Ref
        std::shared_ptr<RendererAPI> GetRendererAPI() { return m_rendererAPI; }

    private:
        void SubmitMeshImpl() {};

    private:
        static RenderSystem* m_renderer;

    private:
        RenderCommandQueue m_commandQueue;
        std::unique_ptr<ShaderLibrary> m_shaderLibrary;
        std::shared_ptr<RendererAPI> m_rendererAPI;


        struct SceneData {
            glm::mat4 viewProjectionMatrix;
        };
        static SceneData* m_sceneData;

    };
}