#pragma once

template <typename... Args>
void imgui_bind_property(const char* name, Args&&...) {
    ImGui::TextDisabled("couldn't bind \"%s\"", name);
}

inline void imgui_bind_property(const char* name, bool& data) { ImGui::Checkbox(name, &data); }
inline void imgui_bind_property(const char* name, int& data) { ImGui::DragInt(name, &data); }
inline void imgui_bind_property(const char* name, float& data) { ImGui::DragFloat(name, &data); }
//inline void imgui_bind_property(const char* name, double& data); // no DragDouble...

inline void imgui_bind_property(const char* name, glm::vec3& data, int* = nullptr) {
    ImGui::DragFloat3(name, (float*)&data);
}

inline void imgui_bind_property(const char* name, glm::quat& data, int* = nullptr) {
    ImGui::DragFloat4(name, (float*)&data);
}
