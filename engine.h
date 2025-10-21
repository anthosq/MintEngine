#pragma once

#include "src/Application.h"
#include "src/Core.h"
#include "src/global_context.h"
#include "src/log_system.h"
#include "src/imgui/imgui_layer.h"
#include "src/keycodes.h"
#include "src/mousecodes.h"
#include "src/input.h"
#include "src/core/time_step.h"

#include "src/event/event_system.h"
#include "src/event/application_event.h"
#include "src/event/key_event.h"
#include "src/event/mouse_event.h"

// render related
#include "src/render/render_system.h"
#include "src/render/render_command.h"
#include "src/render/shader.h"
#include "src/render/buffer.h"
#include "src/render/vertex_array.h"
#include "src/render/camera.h"

// Entry point
#include "src/EntryPoint.h"
// ----- -----