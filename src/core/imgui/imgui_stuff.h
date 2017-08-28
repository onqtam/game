#pragma once

#include "core/tags.h"

template <typename... Args>
const char* imgui_bind_attribute(Object&, const char*, const char* attr, Args&&...) {
    ImGui::TextDisabled("couldn't bind \"%s\"", attr);
    return nullptr;
}

HAPI const char* imgui_bind_attribute(Object& e, const char* mixin, const char* attr, bool& data);
HAPI const char* imgui_bind_attribute(Object& e, const char* mixin, const char* attr, int& data);
HAPI const char* imgui_bind_attribute(Object& e, const char* mixin, const char* attr, float& data);
HAPI const char* imgui_bind_attribute(Object& e, const char* mixin, const char* attr, double& data);

HAPI const char* imgui_bind_attribute(Object& e, const char* mixin, const char* attr, std::string& data);

#ifdef _WIN32
HAPI const char* imgui_bind_file_popup(Object& e, const char* mixin, const char* attr, std::string& data, const char* filters);
inline const char* imgui_bind_attribute(Object& e, const char* mixin, const char* attr, std::string& data, tag::image) {
    return imgui_bind_file_popup(e, mixin, attr, data, "jpg,png;psd"); // default is for jpg and png files. Second is for psd files.
}
inline const char* imgui_bind_attribute(Object& e, const char* mixin, const char* attr, std::string& data, tag::mesh) {
    return imgui_bind_file_popup(e, mixin, attr, data, "bin");
}
#endif // _WIN32

HAPI const char* imgui_bind_attribute(Object& e, const char* mixin, const char* attr, glm::vec3& data);
HAPI const char* imgui_bind_attribute(Object& e, const char* mixin, const char* attr, glm::quat& data);
