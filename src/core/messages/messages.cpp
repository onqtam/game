#include "messages_camera.h"
#include "messages_rendering.h"
#include "messages_editor.h"

// common
HA_DEFINE_MSG(common, serialize_mixins)
HA_DEFINE_MSG(common, deserialize_mixins)
HA_DEFINE_MSG(common, get_imgui_binding_callbacks_from_mixins)

// other
DYNAMIX_DEFINE_MESSAGE(no_gizmo);

// camera
HA_DEFINE_MSG(cam, get_view_matrix)
HA_DEFINE_MSG(cam, get_projection_matrix)

// rendering
HA_DEFINE_MSG(rend, get_rendering_parts)
HA_DEFINE_MSG(rend, get_aabb)

// editor
HA_DEFINE_MSG(edit, add_changed_attribute)
