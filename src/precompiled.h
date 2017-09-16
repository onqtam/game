#pragma once

#include <cmath>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <cstddef>
#include <cctype>
#include <ciso646>

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

#include "utils/suppress_warnings.h"
#include "utils/visibility.h"

HA_SUPPRESS_WARNINGS

#define DYNAMIX_NO_DM_THIS
#include <dynamix/dynamix.hpp>

#include <ppk_assert.h>
#define hassert PPK_ASSERT

#include <sajson/include/sajson.h>

#define BGFX_SHARED_LIB_USE 1
#include <bgfx/c99/bgfx.h>
#define BGFX_INVALID_HANDLE UINT16_MAX

#define IMGUI_API HA_SYMBOL_IMPORT
#include <imgui.h>

#include <yama/yama.hpp>

#include <boost/variant.hpp>

#include <boost/mpl/for_each.hpp>

HA_SUPPRESS_WARNINGS_END

#include "utils/types.h"
#include "utils/preprocessor.h"
#include "utils/doctest/doctest_proxy.h"
#include "utils/singleton.h"
#include "utils/transform.h"
#include "core/messages/message_macros.h"
#include "core/serialization/JsonData.h"
#include "core/tags.h"
#include "core/Object.h"
#include "core/messages/messages_common.h"
#include "core/registry/registry.h"
