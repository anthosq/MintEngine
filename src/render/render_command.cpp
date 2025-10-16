#include "render_command.h"
#include "render/interface/opengl/opengl_renderer_api.h"

namespace Mint {
    RendererAPI* RenderCommand::s_renderer_api = new OpenGLRendererAPI();
}