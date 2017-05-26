#pragma once

#include "utils/suppress_warnings.h"
#include "core/serialization/JsonData.h"

HARDLY_SUPPRESS_WARNINGS

#include <dynamix/dynamix.hpp>
#include <sajson/include/sajson.h>
#include <dynamix/gen/no_arity_message_macros.hpp>

HARDLY_SUPPRESS_WARNINGS_END

#include "utils/visibility.h"
#ifdef BUILDING_MESSAGES
#define MESSAGES_PUBLIC SYMBOL_EXPORT
#else
#define MESSAGES_PUBLIC SYMBOL_IMPORT
#endif

HARDLY_SUPPRESS_WARNINGS

DYNAMIX_EXPORTED_CONST_MULTICAST_MESSAGE_1(MESSAGES_PUBLIC, void, trace, std::ostream&, o)

DYNAMIX_EXPORTED_CONST_MULTICAST_MESSAGE_1(MESSAGES_PUBLIC, void, serialize, JsonData&, out)
DYNAMIX_EXPORTED_MULTICAST_MESSAGE_1(MESSAGES_PUBLIC, void, deserialize, const sajson::value&, in)

DYNAMIX_EXPORTED_MESSAGE_1(MESSAGES_PUBLIC, void, set_id, int, id)
DYNAMIX_EXPORTED_CONST_MESSAGE_0(MESSAGES_PUBLIC, int, get_id)

DYNAMIX_EXPORTED_MESSAGE_1(MESSAGES_PUBLIC, void, add_value, int, value)
DYNAMIX_EXPORTED_CONST_MESSAGE_0(MESSAGES_PUBLIC, int, get_value)

HARDLY_SUPPRESS_WARNINGS_END
