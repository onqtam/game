#pragma once

template <typename... Args>
void imgui_bind_property(const char* name, Args&&...) {
    ImGui::TextDisabled("couldn't bind \"%s\"", name);
}

inline void imgui_bind_property(const char* name, float& data) { ImGui::DragFloat(name, &data, 0.1f); }
inline void imgui_bind_property(const char* name, glm::vec3& data, int* = nullptr) {
    ImGui::DragFloat3(name, (float*)&data, 0.1f);
}
