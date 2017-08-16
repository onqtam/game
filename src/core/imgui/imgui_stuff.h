#pragma once

template <typename... Args>
void imgui_bind_attribute(Object&, const char*, const char* prop, Args&&...) {
    ImGui::TextDisabled("couldn't bind \"%s\"", prop);
}

HAPI void imgui_bind_attribute(Object& e, const char* mixin_name, const char* prop, bool& data);
HAPI void imgui_bind_attribute(Object& e, const char* mixin_name, const char* prop, int& data);
HAPI void imgui_bind_attribute(Object& e, const char* mixin_name, const char* prop, float& data);
HAPI void imgui_bind_attribute(Object& e, const char* mixin_name, const char* prop, double& data);

HAPI void imgui_bind_attribute(Object& e, const char* mixin_name, const char* prop, std::string& data);

HAPI void imgui_bind_attribute(Object& e, const char* mixin_name, const char* prop, glm::vec3& data);
HAPI void imgui_bind_attribute(Object& e, const char* mixin_name, const char* prop, glm::quat& data);
