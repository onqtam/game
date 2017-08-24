#pragma once

#include "core/tags.h"

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
HAPI const char* imgui_bind_file_popup(Object& e, const char* mixin, const char* prop, std::string& data, const char* filters);
#endif // _WIN32

inline const char* imgui_bind_attribute(Object& e, const char* mixin, const char* prop, std::string& data, tag::image) {
    return imgui_bind_file_popup(e, mixin, prop, data, "jpg,png;psd"); // default is for jpg and png files. Second is for psd files.
}
inline const char* imgui_bind_attribute(Object& e, const char* mixin, const char* prop, std::string& data, tag::mesh) {
    return imgui_bind_file_popup(e, mixin, prop, data, "bin");
}

HAPI const char* imgui_bind_attribute(Object& e, const char* mixin, const char* prop, glm::vec3& data);
HAPI const char* imgui_bind_attribute(Object& e, const char* mixin, const char* prop, glm::quat& data);
