#pragma once

#include "JsonData.h"

#include "utils/visibility.h"

#include "utils/suppress_warnings.h"
#include "utils/glm_proxy.h"

HARDLY_SUPPRESS_WARNINGS
#include <sajson/include/sajson.h>
HARDLY_SUPPRESS_WARNINGS_END

#include <string>
#include <vector>
#include <list>
#include <map>

HAPI void serialize(int data, JsonData& out);
HAPI void serialize(float data, JsonData& out);
HAPI void serialize(double data, JsonData& out);
HAPI void serialize(bool data, JsonData& out);

HAPI void deserialize(int& data, const sajson::value& val);
HAPI void deserialize(float& data, const sajson::value& val);
HAPI void deserialize(double& data, const sajson::value& val);
HAPI void deserialize(bool& data, const sajson::value& val);

HAPI void serialize(const glm::vec3& data, JsonData& out);
HAPI void deserialize(glm::vec3& data, const sajson::value& val);
