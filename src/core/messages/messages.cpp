#include "messages.h"
#include "messages_camera.h"
#include "messages_rendering.h"
#include "messages_editor.h"

// common
HA_DEFINE_MSG(common, serialize_mixins)
HA_DEFINE_MSG(common, deserialize_mixins)
HA_DEFINE_MSG(common, set_attribute_mixins)
HA_DEFINE_MSG(common, imgui_bind_attributes_mixins)

// transform
HA_DEFINE_MSG(tr, set_pos)
HA_DEFINE_MSG(tr, set_scl)
HA_DEFINE_MSG(tr, set_rot)
HA_DEFINE_MSG(tr, get_pos)
HA_DEFINE_MSG(tr, get_scl)
HA_DEFINE_MSG(tr, get_rot)
HA_DEFINE_MSG(tr, set_transform)
HA_DEFINE_MSG(tr, get_transform)
HA_DEFINE_MSG(tr, get_transform_mat)
HA_DEFINE_MSG(tr, move)

// parental
DYNAMIX_DEFINE_MESSAGE(get_parent);
DYNAMIX_DEFINE_MESSAGE(get_children);
DYNAMIX_DEFINE_MESSAGE(set_parent);

// selected
HA_DEFINE_MSG(sel, no_gizmo)
HA_DEFINE_MSG(sel, get_transform_on_gizmo_start)

// camera
HA_DEFINE_MSG(cam, get_view_matrix)
HA_DEFINE_MSG(cam, get_projection_matrix)

// rendering
HA_DEFINE_MSG(rend, get_rendering_parts)
HA_DEFINE_MSG(rend, get_aabb)

// editor
HA_DEFINE_MSG(edit, add_changed_attribute)
