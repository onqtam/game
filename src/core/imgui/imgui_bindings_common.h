#pragma once

template <typename... Args>
cstr imgui_bind_attribute(Object&, cstr, cstr attr, Args&&...) {
    ImGui::TextDisabled("couldn't bind \"%s\"", attr);
    return nullptr;
}

HAPI cstr imgui_bind_attribute(Object& e, cstr mixin, cstr attr, bool& data);
HAPI cstr imgui_bind_attribute(Object& e, cstr mixin, cstr attr, int& data);
HAPI cstr imgui_bind_attribute(Object& e, cstr mixin, cstr attr, float& data);
HAPI cstr imgui_bind_attribute(Object& e, cstr mixin, cstr attr, double& data);

HAPI cstr imgui_bind_attribute(Object& e, cstr mixin, cstr attr, std::string& data);

#ifdef _WIN32
HAPI cstr imgui_bind_file_popup(Object& e, cstr mixin, cstr attr, std::string& data, cstr filters);
inline cstr imgui_bind_attribute(Object& e, cstr mixin, cstr attr, std::string& data, tag::image) {
    return imgui_bind_file_popup(e, mixin, attr, data, "jpg,png;psd");
}
inline cstr imgui_bind_attribute(Object& e, cstr mixin, cstr attr, std::string& data, tag::mesh) {
    return imgui_bind_file_popup(e, mixin, attr, data, "bin");
}
#endif // _WIN32

HAPI cstr imgui_bind_attribute(Object& e, cstr mixin, cstr attr, yama::vector3& data);
HAPI cstr imgui_bind_attribute(Object& e, cstr mixin, cstr attr, yama::quaternion& data);
//HAPI cstr imgui_bind_attribute(Object& e, cstr mixin, cstr attr, transform& data);
