#pragma once

template <typename... Args>
void imgui_bind_property(Entity&, const char* name, Args&&...) {
    ImGui::TextDisabled("couldn't bind \"%s\"", name);
}

HAPI void imgui_bind_property(Entity& e, const char* name, bool& data);
HAPI void imgui_bind_property(Entity& e, const char* name, int& data);
HAPI void imgui_bind_property(Entity& e, const char* name, float& data);
//HAPI void imgui_bind_property(Entity& e, const char* name, double& data); // no DragDouble...

HAPI void imgui_bind_property(Entity& e, const char* name, std::string& data);

HAPI void imgui_bind_property(Entity& e, const char* name, glm::vec3& data);
HAPI void imgui_bind_property(Entity& e, const char* name, glm::quat& data);
