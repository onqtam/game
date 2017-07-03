#include "messages.h"
#include "messages_camera.h"
#include "messages_rendering.h"

// common
DYNAMIX_DEFINE_MESSAGE(imgui_bind_properties);
DYNAMIX_DEFINE_MESSAGE(serialize);
DYNAMIX_DEFINE_MESSAGE(deserialize);

// event
DYNAMIX_DEFINE_MESSAGE(process_event);

// transform
DYNAMIX_DEFINE_MESSAGE(set_pos);
DYNAMIX_DEFINE_MESSAGE(set_scl);
DYNAMIX_DEFINE_MESSAGE(set_rot);
DYNAMIX_DEFINE_MESSAGE(get_pos);
DYNAMIX_DEFINE_MESSAGE(get_scl);
DYNAMIX_DEFINE_MESSAGE(get_rot);
DYNAMIX_DEFINE_MESSAGE(get_model_transform);
DYNAMIX_DEFINE_MESSAGE(move);

DYNAMIX_DEFINE_MESSAGE(get_parent);
DYNAMIX_DEFINE_MESSAGE(get_children);
DYNAMIX_DEFINE_MESSAGE(set_parent);
DYNAMIX_DEFINE_MESSAGE(add_child);
DYNAMIX_DEFINE_MESSAGE(remove_child);

// camera
DYNAMIX_DEFINE_MESSAGE(get_view_matrix);
DYNAMIX_DEFINE_MESSAGE(get_projection_matrix);

// rendering
DYNAMIX_DEFINE_MESSAGE(get_rendering_parts);
