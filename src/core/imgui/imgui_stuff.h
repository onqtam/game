#pragma once

#include "core/tags.h"

typedef std::string mesh_path;

template <typename... Args>
const char* imgui_bind_attribute(Object&, const char*, const char* prop, Args&&...) {
    ImGui::TextDisabled("couldn't bind \"%s\"", prop);
    return nullptr;
}

HAPI const char* imgui_bind_attribute(Object& e, const char* mixin, const char* prop, bool& data);
HAPI const char* imgui_bind_attribute(Object& e, const char* mixin, const char* prop, int& data);
HAPI const char* imgui_bind_attribute(Object& e, const char* mixin, const char* prop, float& data);
HAPI const char* imgui_bind_attribute(Object& e, const char* mixin, const char* prop, double& data);

HAPI const char* imgui_bind_attribute(Object& e, const char* mixin, const char* prop, std::string& data);
#ifdef _WIN32
HAPI const char* imgui_bind_attribute(Object& e, const char* mixin, const char* prop, mesh_path& data);
#endif // _WIN32

HAPI const char* imgui_bind_attribute(Object& e, const char* mixin, const char* prop, glm::vec3& data);
HAPI const char* imgui_bind_attribute(Object& e, const char* mixin, const char* prop, glm::quat& data);
