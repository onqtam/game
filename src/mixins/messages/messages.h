#pragma once

#include "messages_common.h"

#include "core/serialization/JsonData.h"
#include "core/InputEvent.h"

#include "utils/glm_proxy.h"

HARDLY_SUPPRESS_WARNINGS

DYNAMIX_EXPORTED_CONST_MULTICAST_MESSAGE_1(MESSAGES_PUBLIC, void, trace, std::ostream&, o)

DYNAMIX_EXPORTED_CONST_MULTICAST_MESSAGE_1(MESSAGES_PUBLIC, void, serialize, JsonData&, out)
DYNAMIX_EXPORTED_MULTICAST_MESSAGE_1(MESSAGES_PUBLIC, void, deserialize, const sajson::value&, in)

DYNAMIX_EXPORTED_MESSAGE_1(MESSAGES_PUBLIC, void, process_event, const InputEvent&, ev)

DYNAMIX_EXPORTED_MESSAGE_1(MESSAGES_PUBLIC, void, set_id, int, id)
DYNAMIX_EXPORTED_CONST_MESSAGE_0(MESSAGES_PUBLIC, int, get_id)

DYNAMIX_EXPORTED_MESSAGE_1(MESSAGES_PUBLIC, void, set_pos, const glm::vec3&, pos)
DYNAMIX_EXPORTED_CONST_MESSAGE_0(MESSAGES_PUBLIC, const glm::vec3&, get_pos)

HARDLY_SUPPRESS_WARNINGS_END
