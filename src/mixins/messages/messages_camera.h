#pragma once

#include "messages_common.h"

#include "utils/glm_proxy.h"

HARDLY_SUPPRESS_WARNINGS

DYNAMIX_EXPORTED_MESSAGE_0(MESSAGES_PUBLIC, const glm::mat4&, get_view_matrix)
DYNAMIX_EXPORTED_MESSAGE_0(MESSAGES_PUBLIC, const glm::mat4&, get_projection_matrix)

HARDLY_SUPPRESS_WARNINGS_END
