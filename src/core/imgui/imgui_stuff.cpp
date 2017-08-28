#include "imgui_stuff.h"

#include "utils/utils.h"

#include "core/World.h"
#include "core/serialization/serialization.h"
#include "core/messages/messages_editor.h"

HA_SUPPRESS_WARNINGS
#ifdef _WIN32
#include <nfd.h>
#endif // _WIN32
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

static bool DragFloats(const char* label, float* items, int numItems, bool* pJustReleased = nullptr,
                       bool* pJustActivated = nullptr, float v_speed = 1.0f, float v_min = 0.0f,
                       float v_max = 0.0f, const char* display_format = "%.3f",
                       float power = 1.0f) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if(window->SkipItems)
        return false;

    ImGuiContext& g             = *GImGui;
    bool          value_changed = false;
    ImGui::BeginGroup();
    ImGui::PushID(label);
    PushMultiItemsWidths(numItems);
    for(int i = 0; i < numItems; i++) {
        ImGui::PushID(i);
        value_changed |=
                ImGui::DragFloat("##v", &items[i], v_speed, v_min, v_max, display_format, power);

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

static bool DragInts(const char* label, int* items, int numItems, bool* pJustReleased = nullptr,
                     bool* pJustActivated = nullptr, float v_speed = 1.0f, int v_min = 0,
                     int v_max = 0, const char* display_format = "%.0f") {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if(window->SkipItems)
        return false;

    ImGuiContext& g             = *GImGui;
    bool          value_changed = false;
    ImGui::BeginGroup();
    ImGui::PushID(label);
    PushMultiItemsWidths(numItems);
    for(int i = 0; i < numItems; i++) {
        ImGui::PushID(i);
        value_changed |= ImGui::DragInt("##v", &items[i], v_speed, v_min, v_max, display_format);

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

inline const char* no_prefix(const char* attr) {
    if(attr[0] != '\0' && attr[0] == 'm' && attr[1] != '\0' && attr[1] == '_')
        return attr + 2;
    return attr;
}

template <typename T>
const char* bind_floats(Object& e, const char* mixin, const char* attr, T& data, int num_elements) {
    static T data_when_dragging_started;
    bool     justReleased  = false;
    bool     justActivated = false;
    DragFloats(no_prefix(attr), (float*)&data, num_elements, &justReleased, &justActivated);
    if(justActivated) {
        data_when_dragging_started = data;
    }
    if(justReleased) {
        JsonData old_val = command(mixin, attr, data_when_dragging_started);
        JsonData new_val = command(mixin, attr, data);
        edit::add_changed_attribute(World::get().editor(), e.id(), old_val.data(), new_val.data());
        return attr;
    }
    return nullptr;
}

template <typename T>
const char* bind_ints(Object& e, const char* mixin, const char* attr, T& data, int num_elements) {
    static T data_when_dragging_started;
    bool     justReleased  = false;
    bool     justActivated = false;
    DragInts(no_prefix(attr), (int*)&data, num_elements, &justReleased, &justActivated);
    if(justActivated) {
        data_when_dragging_started = data;
    }
    if(justReleased) {
        JsonData old_val = command(mixin, attr, data_when_dragging_started);
        JsonData new_val = command(mixin, attr, data);
        edit::add_changed_attribute(World::get().editor(), e.id(), old_val.data(), new_val.data());
        return attr;
    }
    return nullptr;
}

const char* imgui_bind_attribute(Object& e, const char* mixin, const char* attr, bool& data) {
    if(ImGui::Checkbox(no_prefix(attr), &data)) {
        JsonData old_val = command(mixin, attr, !data);
        JsonData new_val = command(mixin, attr, data);
        edit::add_changed_attribute(World::get().editor(), e.id(), old_val.data(), new_val.data());
        return attr;
    }
    return nullptr;
}
const char* imgui_bind_attribute(Object& e, const char* mixin, const char* attr, int& data) {
    return bind_ints(e, mixin, attr, data, 1);
}
const char* imgui_bind_attribute(Object& e, const char* mixin, const char* attr, float& data) {
    return bind_floats(e, mixin, attr, data, 1);
}
const char* imgui_bind_attribute(Object& e, const char* mixin, const char* attr, double& data) {
    float temp = static_cast<float>(data);
    auto  res  = bind_floats(e, mixin, attr, temp, 1);
    data       = static_cast<double>(temp);
    return res;
}

const char* imgui_bind_attribute(Object& e, const char* mixin, const char* attr,
                                 std::string& data) {
    static char buf[128] = "";
    Utils::strncpy(buf, data.c_str(), HA_COUNT_OF(buf));
    if(ImGui::InputText(no_prefix(attr), buf, HA_COUNT_OF(buf),
                        ImGuiInputTextFlags_EnterReturnsTrue)) {
        JsonData old_val = command(mixin, attr, data);
        data             = buf;
        JsonData new_val = command(mixin, attr, data);
        edit::add_changed_attribute(World::get().editor(), e.id(), old_val.data(), new_val.data());
        return attr;
    }
    return nullptr;
}

#ifdef _WIN32
const char* imgui_bind_file_popup(Object& e, const char* mixin, const char* attr, std::string& data,
                                  const char* filters) {
    ImGui::PushItemWidth(200);
    ImGui::InputText("##input", data.data(), data.length(), ImGuiInputTextFlags_ReadOnly);
    ImGui::PopItemWidth();
    ImGui::SameLine();

    char buf[200];
    snprintf(buf, HA_COUNT_OF(buf), "browse##%s", no_prefix(attr));

    if(ImGui::Button(buf)) {
        nfdchar_t*  outPath = nullptr;
        nfdresult_t result  = NFD_OpenDialog(filters, nullptr, &outPath);
        hassert(result != NFD_ERROR, "Error: %s\n", NFD_GetError());
        if(result == NFD_OKAY) {
            JsonData old_val = command(mixin, attr, data);
            data             = outPath;
            JsonData new_val = command(mixin, attr, data);
            edit::add_changed_attribute(World::get().editor(), e.id(), old_val.data(),
                                        new_val.data());

            free(outPath);
            return attr;
        }
    }
    ImGui::SameLine();
    ImGui::LabelText("##text", attr);
    return nullptr;
}
#endif // _WIN32

const char* imgui_bind_attribute(Object& e, const char* mixin, const char* attr, glm::vec3& data) {
    return bind_floats(e, mixin, attr, data, 3);
}

const char* imgui_bind_attribute(Object& e, const char* mixin, const char* attr, glm::quat& data) {
    return bind_floats(e, mixin, attr, data, 4);
}
