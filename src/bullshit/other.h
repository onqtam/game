
#include <imgui_internal.h>

bool IsItemActiveLastFrame() {
    ImGuiContext& g = *ImGui::GetCurrentContext();

    if(g.ActiveIdPreviousFrame)
        return g.ActiveIdPreviousFrame == g.CurrentWindow->DC.LastItemId;
    return false;
}

bool IsItemJustReleased() { return IsItemActiveLastFrame() && !ImGui::IsItemActive(); }

bool IsItemJustActivated() { return !IsItemActiveLastFrame() && ImGui::IsItemActive(); }

bool DragFloats(const char* label, float* floats, int numFloats, bool* pJustReleased = nullptr,
                bool* pJustActivated = nullptr, float v_speed = 1.0f, float v_min = 0.0f,
                float v_max = 0.0f, const char* display_format = "%.3f", float power = 1.0f) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if(window->SkipItems)
        return false;

    ImGuiContext& g = *ImGui::GetCurrentContext();
    bool          value_changed = false;
    ImGui::BeginGroup();
    ImGui::PushID(label);
    //PushMultiItemsWidths(numFloats);
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
