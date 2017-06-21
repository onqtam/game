#define BUILDING_MESSAGES
#include "messages.h"
#include "messages_camera.h"

DYNAMIX_DEFINE_MESSAGE(trace);

DYNAMIX_DEFINE_MESSAGE(serialize);
DYNAMIX_DEFINE_MESSAGE(deserialize);

DYNAMIX_DEFINE_MESSAGE(process_event);

DYNAMIX_DEFINE_MESSAGE(set_id);
DYNAMIX_DEFINE_MESSAGE(get_id);

DYNAMIX_DEFINE_MESSAGE(set_pos);
DYNAMIX_DEFINE_MESSAGE(get_pos);

// camera
DYNAMIX_DEFINE_MESSAGE(get_view_matrix);
DYNAMIX_DEFINE_MESSAGE(get_projection_matrix);
