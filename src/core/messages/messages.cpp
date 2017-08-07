#include "messages.h"
#include "messages_camera.h"
#include "messages_rendering.h"
#include "messages_editor.h"

// common
HA_DEFINE_MSG(common, serialize)
HA_DEFINE_MSG(common, deserialize)
HA_DEFINE_MSG(common, imgui_bind_properties)

// transform
HA_DEFINE_MSG(tr, set_pos)
HA_DEFINE_MSG(tr, set_scl)
HA_DEFINE_MSG(tr, set_rot)
HA_DEFINE_MSG(tr, get_pos)
HA_DEFINE_MSG(tr, get_scl)
HA_DEFINE_MSG(tr, get_rot)
HA_DEFINE_MSG(tr, get_model_transform)
HA_DEFINE_MSG(tr, move)

// hierarchical
DYNAMIX_DEFINE_MESSAGE(get_parent);
DYNAMIX_DEFINE_MESSAGE(get_children);
DYNAMIX_DEFINE_MESSAGE(set_parent);
DYNAMIX_DEFINE_MESSAGE(add_child);
DYNAMIX_DEFINE_MESSAGE(remove_child);

// selected
HA_DEFINE_MSG(sel, get_gizmo_transform)
HA_DEFINE_MSG(sel, get_last_stable_gizmo_transform)
HA_DEFINE_MSG(sel, no_gizmo)

// camera
HA_DEFINE_MSG(cam, get_view_matrix)
HA_DEFINE_MSG(cam, get_projection_matrix)

// rendering
HA_DEFINE_MSG(rend, get_rendering_parts)
HA_DEFINE_MSG(rend, get_aabb)

// editor
HA_DEFINE_MSG(edit, add_changed_property)
//HA_DEFINE_MSG(edit, add_change_started_data)
//HA_DEFINE_MSG(edit, get_change_started_data)
