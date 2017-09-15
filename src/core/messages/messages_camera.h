#pragma once

const yama::vector3 k_init_look_direction = yama::normalize(yama::v(0, -1, -0.2f));
const yama::vector3 k_forward             = {0, 0, -1};

HA_SUPPRESS_WARNINGS

HA_MSG_0(cam, yama::matrix, get_view_matrix)
HA_MSG_0(cam, yama::matrix, get_projection_matrix)

#define Interface_camera cam::get_view_matrix_msg& cam::get_projection_matrix_msg

HA_SUPPRESS_WARNINGS_END
