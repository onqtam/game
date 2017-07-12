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
#include "utils/preprocessor.h"
#include "utils/doctest/doctest_proxy.h"

HA_SUPPRESS_WARNINGS

#define DYNAMIX_NO_DM_THIS
#include <dynamix/dynamix.hpp>

#include <ppk_assert.h>
#define hassert PPK_ASSERT

#ifdef _MSC_VER
// oddly enough these are not silenced by HA_SUPPRESS_WARNINGS and have to be listed explicitly
#pragma warning(disable : 4702) // unreachable code
#pragma warning(disable : 4715) // not all control paths return a value
#endif                          // _MSC_VER
#include <sajson/include/sajson.h>

#define BGFX_SHARED_LIB_USE 1
#include <bgfx/c99/bgfx.h>
#define BGFX_INVALID_HANDLE UINT16_MAX

#define IMGUI_API HA_SYMBOL_IMPORT
#include <imgui.h>

#define GLM_FORCE_INLINE
#define GLM_FORCE_SWIZZLE
#define GLM_FORCE_CXX03
#define GLM_FORCE_PURE
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_NO_CTOR_INIT
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/norm.hpp>

HA_SUPPRESS_WARNINGS_END

using glm::int8;
using glm::int16;
using glm::int32;
using glm::int64;
using glm::uint8;
using glm::uint16;
using glm::uint32;
using glm::uint64;

#include "utils/singleton.h"
#include "core/Entity.h"
