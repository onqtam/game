#pragma once

#include "suppress_warnings.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_INLINE
#define GLM_FORCE_SWIZZLE
#define GLM_FORCE_CXX03
#define GLM_FORCE_PURE
#define GLM_ENABLE_EXPERIMENTAL
//#define GLM_MESSAGES

HARDLY_SUPPRESS_WARNINGS

#include "D:/game/third_party/glm/glm/glm.hpp"
#include "D:/game/third_party/glm/glm/gtc/quaternion.hpp"
#include "D:/game/third_party/glm/glm/gtc/matrix_access.hpp"
#include "D:/game/third_party/glm/glm/gtc/matrix_transform.hpp"
#include "D:/game/third_party/glm/glm/gtc/type_ptr.hpp"
#include "D:/game/third_party/glm/glm/gtx/norm.hpp"

HARDLY_SUPPRESS_WARNINGS_END

using glm::int8;
using glm::int16;
using glm::int32;
using glm::int64;
using glm::uint8;
using glm::uint16;
using glm::uint32;
using glm::uint64;
