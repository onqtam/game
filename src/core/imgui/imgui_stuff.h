#pragma once

HAPI bool DragFloats(const char* label, float* floats, int numFloats, bool* pJustReleased = nullptr,
                     bool* pJustActivated = nullptr, float v_speed = 1.0f, float v_min = 0.0f,
                     float v_max = 0.0f, const char* display_format = "%.3f", float power = 1.0f);

template <typename... Args>
void imgui_bind_property(const char* name, Args&&...) {
    ImGui::TextDisabled("couldn't bind \"%s\"", name);
}

HAPI void imgui_bind_property(const char* name, bool& data);
HAPI void imgui_bind_property(const char* name, int& data);
HAPI void imgui_bind_property(const char* name, float& data);
//HAPI void imgui_bind_property(const char* name, double& data); // no DragDouble...

HAPI void imgui_bind_property(const char* name, std::string& data);

HAPI void imgui_bind_property(const char* name, glm::vec3& data);
HAPI void imgui_bind_property(const char* name, glm::quat& data);
