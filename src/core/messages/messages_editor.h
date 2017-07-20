#pragma once

#include "core/serialization/JsonData.h"

HA_SUPPRESS_WARNINGS

HA_MSG_1(edit, void, add_change_started_data, int, a)
HA_MSG_1(edit, void, get_change_started_data, int, a)
HA_MSG_3(edit, void, add_changed_property, eid, e, const std::string&, prop, const std::vector<char>&, json)

#define Interface_editor edit::add_changed_property_msg

HA_SUPPRESS_WARNINGS_END
