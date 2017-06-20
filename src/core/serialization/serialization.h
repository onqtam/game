#pragma once

#include "JsonData.h"

#include "utils/visibility.h"
#ifdef BUILDING_SERIALIZATION
#define SERIALIZATION_PUBLIC SYMBOL_EXPORT
#else
#define SERIALIZATION_PUBLIC SYMBOL_IMPORT
#endif

#include "utils/suppress_warnings.h"

HARDLY_SUPPRESS_WARNINGS
#include <sajson/include/sajson.h>
#include <glm/glm/glm.hpp>
HARDLY_SUPPRESS_WARNINGS_END

#include <string>
#include <vector>
#include <list>
#include <map>

SERIALIZATION_PUBLIC void serialize(int data, JsonData& out);
SERIALIZATION_PUBLIC void serialize(float data, JsonData& out);
SERIALIZATION_PUBLIC void serialize(double data, JsonData& out);
SERIALIZATION_PUBLIC void serialize(bool data, JsonData& out);

SERIALIZATION_PUBLIC void deserialize(int& data, const sajson::value& val);
SERIALIZATION_PUBLIC void deserialize(float& data, const sajson::value& val);
SERIALIZATION_PUBLIC void deserialize(double& data, const sajson::value& val);
SERIALIZATION_PUBLIC void deserialize(bool& data, const sajson::value& val);

SERIALIZATION_PUBLIC void serialize(const glm::vec3& data, JsonData& out);
SERIALIZATION_PUBLIC void deserialize(glm::vec3& data, const sajson::value& val);
