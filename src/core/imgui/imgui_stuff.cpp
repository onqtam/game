#include "imgui_stuff.h"

#include "utils/utils.h"

#include "core/World.h"
#include "core/serialization/serialization.h"
#include "core/messages/messages_editor.h"

HA_SUPPRESS_WARNINGS
#include <imgui/imgui_internal.h>
HA_SUPPRESS_WARNINGS_END

// CAUTION: Duplicated form ImGui in order to implement widgets that can tell if the user is done interacting.
static void PushMultiItemsWidths(int components, float w_full = 0.0f);
// clang-format off
static void PushMultiItemsWidths(int components, float w_full)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    const ImGuiStyle& style = GImGui->Style;
    if (w_full <= 0.0f)
        w_full = ImGui::CalcItemWidth();
    const float w_item_one  = ImMax(1.0f, (float)(int)((w_full - (style.ItemInnerSpacing.x) * (components-1)) / (float)components));
    const float w_item_last = ImMax(1.0f, (float)(int)(w_full - (w_item_one + style.ItemInnerSpacing.x) * (components-1)));
    window->DC.ItemWidthStack.push_back(w_item_last);
    for (int i = 0; i < components-1; i++)
        window->DC.ItemWidthStack.push_back(w_item_one);
    window->DC.ItemWidth = window->DC.ItemWidthStack.back();
}
// clang-format on

static bool IsItemActiveLastFrame() {
    ImGuiContext& g = *GImGui;

    if(g.ActiveIdPreviousFrame)
        return g.ActiveIdPreviousFrame == g.CurrentWindow->DC.LastItemId;
    return false;
}

static bool IsItemJustReleased() { return IsItemActiveLastFrame() && !ImGui::IsItemActive(); }
static bool IsItemJustActivated() { return !IsItemActiveLastFrame() && ImGui::IsItemActive(); }

static bool DragFloats(const char* label, float* floats, int numFloats,
                       bool* pJustReleased = nullptr, bool* pJustActivated = nullptr,
                       float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f,
                       const char* display_format = "%.3f", float power = 1.0f) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if(window->SkipItems)
        return false;

    ImGuiContext& g             = *GImGui;
    bool          value_changed = false;
    ImGui::BeginGroup();
    ImGui::PushID(label);
    PushMultiItemsWidths(numFloats);
    for(int i = 0; i < numFloats; i++) {
        ImGui::PushID(i);
        value_changed |=
                ImGui::DragFloat("##v", &floats[i], v_speed, v_min, v_max, display_format, power);

        if(pJustReleased) {
            *pJustReleased |= IsItemJustReleased();
        }

        if(pJustActivated) {
            *pJustActivated |= IsItemJustActivated();
        }

        ImGui::PopID();
        ImGui::SameLine(0, g.Style.ItemInnerSpacing.x);
        ImGui::PopItemWidth();
    }
    ImGui::PopID();

    ImGui::TextUnformatted(label, ImGui::FindRenderedTextEnd(label));
    ImGui::EndGroup();

    return value_changed;
}

template <typename T>
JsonData construct_undo_redo_command(const char* mixin_name, const char* prop, const T& data) {
    JsonData out;
    out.startObject();
    out.append("\"");
    out.append(mixin_name, strlen(mixin_name));
    out.append("\":");
    out.startObject();
    out.append("\"");
    out.append(prop, strlen(prop));
    out.append("\":");
    serialize(data, out);
    out.endObject();
    out.endObject();

    return out;
}

template <typename T>
void bind_floats(Entity& e, const char* mixin_name, const char* prop, T& data, int num_floats) {
    static T persistent_data;
    bool     justReleased  = false;
    bool     justActivated = false;
    DragFloats(prop, (float*)&data, num_floats, &justReleased, &justActivated);
    if(justActivated) {
        persistent_data = data;
    }
    if(justReleased) {
        JsonData old_val = construct_undo_redo_command(mixin_name, prop, persistent_data);
        JsonData new_val = construct_undo_redo_command(mixin_name, prop, data);
        edit::add_changed_property(World::get().editor(), e.id(), old_val.data(), new_val.data());
    }
}

void imgui_bind_property(Entity& e, const char* mixin_name, const char* prop, bool& data) {
    if(ImGui::Checkbox(prop, &data)) {
        JsonData old_val = construct_undo_redo_command(mixin_name, prop, !data);
        JsonData new_val = construct_undo_redo_command(mixin_name, prop, data);
        edit::add_changed_property(World::get().editor(), e.id(), old_val.data(), new_val.data());
    }
}
void imgui_bind_property(Entity& e, const char* mixin_name, const char* prop, int& data) {
    ImGui::DragInt(prop, &data);
}

void imgui_bind_property(Entity& e, const char* mixin_name, const char* prop, float& data) {
    bind_floats(e, mixin_name, prop, data, 1);
}
void imgui_bind_property(Entity& e, const char* mixin_name, const char* prop, double& data) {
    float temp = static_cast<float>(data);
    bind_floats(e, mixin_name, prop, temp, 1);
    data = temp;
}

void imgui_bind_property(Entity& e, const char* mixin_name, const char* prop, std::string& data) {
    static char buf[128] = "";
    Utils::strncpy(buf, data.c_str(), HA_COUNT_OF(buf));
    if(ImGui::InputText(prop, buf, HA_COUNT_OF(buf), ImGuiInputTextFlags_EnterReturnsTrue)) {
        JsonData old_val = construct_undo_redo_command(mixin_name, prop, data);
        data             = buf;
        JsonData new_val = construct_undo_redo_command(mixin_name, prop, data);
        edit::add_changed_property(World::get().editor(), e.id(), old_val.data(), new_val.data());
    }
}

void imgui_bind_property(Entity& e, const char* mixin_name, const char* prop, glm::vec3& data) {
    // TODO: integrate into undo/redo queue! currently pos/scale work because of a coincidence in editor.cpp and unconsumed input by imgui
    DragFloats(prop, (float*)&data, 3);
}

void imgui_bind_property(Entity& e, const char* mixin_name, const char* prop, glm::quat& data) {
    bind_floats(e, mixin_name, prop, data, 4);
}
