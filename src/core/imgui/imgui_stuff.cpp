#include "imgui_stuff.h"

#include "utils/utils.h"

#include "core/World.h"
#include "core/serialization/serialization.h"
#include "core/messages/messages_editor.h"

#include <imgui/imgui_internal.h>

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

void imgui_bind_property(Entity& e, const char* name, bool& data) {
    if(ImGui::Checkbox(name, &data)) {
        JsonData json;
        json.startObject();
        json.append("\"old\":");
        serialize(!data, json);
        json.addComma();
        json.append("\"new\":");
        serialize(data, json);
        json.endObject();

        edit::add_changed_property(World::get().editor(), e.id(), name, json.data());
        printf("CHANGED!\n");
    }
}
void imgui_bind_property(Entity& e, const char* name, int& data) { ImGui::DragInt(name, &data); }
void imgui_bind_property(Entity& e, const char* name, float& data) { ImGui::DragFloat(name, &data); }

void imgui_bind_property(Entity& e, const char* name, std::string& data) {
    static char buf[128] = "";
    Utils::strncpy(buf, data.c_str(), HA_COUNT_OF(buf));
    if(ImGui::InputText(name, buf, HA_COUNT_OF(buf), ImGuiInputTextFlags_EnterReturnsTrue)) {
        data = buf;
        printf("CHANGED!\n");
    }
}

void imgui_bind_property(Entity& e, const char* name, glm::vec3& data) {
    bool justReleased = false;
    printf("%f %f %f\n", data.x, data.y, data.z);
    DragFloats(name, (float*)&data, 3, &justReleased);
    if(justReleased) {
        printf("CHANGED!\n");
    }
}

void imgui_bind_property(Entity& e, const char* name, glm::quat& data) {
    ImGui::DragFloat4(name, (float*)&data);
}
