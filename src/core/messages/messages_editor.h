#pragma once

#include "core/serialization/JsonData.h"

HA_SUPPRESS_WARNINGS

HA_MSG_3(edit, void, add_changed_attribute, oid, e, const json_buf&, old_val, const json_buf&,
         new_val)
//HA_MSG_3(edit, void, add_change_started_data, oid, e, const std::string&, prop, const boost::any&, val)
//HA_MSG_2(edit, const boost::any&, get_change_started_data, oid, e, const std::string&, prop)

#define Interface_editor edit::add_changed_attribute_msg
//& edit::add_change_started_data& edit::get_change_started_data

HA_SUPPRESS_WARNINGS_END
