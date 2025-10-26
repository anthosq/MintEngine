#pragma once

#include "Application.h"
#include "Core.h"
#include "global_context.h"
#include "log_system.h"
#include "imgui/imgui_layer.h"
#include "keycodes.h"
#include "mousecodes.h"
#include "input.h"
#include "core/time_step.h"

#include "event/event_system.h"
#include "event/application_event.h"
#include "event/key_event.h"
#include "event/mouse_event.h"

// render related
#include "render/render_system.h"
#include "render/render_command.h"
#include "render/shader.h"
#include "render/buffer.h"
#include "render/vertex_array.h"
#include "render/camera.h"
#include "render/texture.h"


// Entry point
#include "EntryPoint.h"
// ----- -----