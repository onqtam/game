#pragma once

#include "utils/suppress_warnings.h"

HARDLY_SUPPRESS_WARNINGS
#include <dynamix/dynamix.hpp>
//#include <dynamix/gen/no_arity_message_macros.hpp>
HARDLY_SUPPRESS_WARNINGS_END

#include "utils/visibility.h"
#ifdef BUILDING_MESSAGES
#define MESSAGES_PUBLIC SYMBOL_EXPORT
#else
#define MESSAGES_PUBLIC SYMBOL_IMPORT
#endif
