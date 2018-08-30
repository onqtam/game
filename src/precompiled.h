#pragma once

#include <cmath>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <cstddef>
#include <cctype>
#include <ciso646>

#include <typeinfo>
#include <functional>
#include <exception>
#include <stdexcept>
#include <iterator>
#include <limits>
#include <numeric>
#include <string>
#include <utility>
#include <memory>
#include <tuple>
#include <new>
#include <random>
#include <chrono>
#include <type_traits>

#include <vector>
#include <array>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <variant>

#include "utils/suppress_warnings.h"
#include "utils/visibility.h"

HA_SUPPRESS_WARNINGS

#define DYNAMIX_NO_DM_THIS
#include <dynamix/dynamix.hpp>
typedef dynamix::internal::mixin_type_info mixin_type_info;

#define PPK_ASSERT_FUNCSPEC HA_SYMBOL_IMPORT
#include <ppk_assert.h>
#define hassert PPK_ASSERT

#include <sajson/include/sajson.h>

#if defined(_WIN32)
#    include <GL/glew.h>
#else
#    define GL_GLEXT_PROTOTYPES
#    include <GL/gl.h>
#    include <GL/glext.h>
#endif

#define IMGUI_API HA_SYMBOL_IMPORT

#include <yama/yama.hpp>

HA_SUPPRESS_WARNINGS_END

#include "utils/preprocessor.h"
#include "utils/doctest/doctest_proxy.h"
#include "utils/types.h"
#include "utils/JsonData.h"
#include "utils/singleton.h"
#include "utils/transform.h"
#include "core/messages/message_macros.h"
#include "core/tags.h"
#include "core/Object.h"
#include "core/messages/messages_common.h"
#include "core/registry/registry.h"
