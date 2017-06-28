#include "common_gen.h"

#include "core/registry/registry.h"
#include "core/ObjectManager.h"

#include "core/messages/messages.h"

#include <iostream>

#include <imgui_internal.h>

using namespace dynamix;
//using namespace std;

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

template <typename... Args>
void imgui_bind_property(const char* name, Args&&...) {
    ImGui::TextDisabled("couldn't bind \"%s\"", name);
}

void imgui_bind_property(const char* name, float& data) { ImGui::DragFloat(name, &data, 0.1f); }
void imgui_bind_property(const char* name, glm::vec3& data, int* = nullptr) {
    ImGui::DragFloat3(name, (float*)&data, 0.1f);
}

class common : public common_gen
{
    HA_MESSAGES_IN_MIXIN(common)
public:
    void trace(std::ostream& o) const { o << " object with id " << int(ha_this.id()) << std::endl; }

    void set_pos(const glm::vec3& in) { pos = in; }
    void move(const glm::vec3& in) { pos += in; }
    const glm::vec3&           get_pos() const { return pos; }

    void imgui_properties() {
        if(ImGui::TreeNode("common")) {
            imgui_bind_property("pos", pos);
            ImGui::TreePop();
        }
    }

    //void select(bool _in) { selected = _in; }
};

HA_MIXIN_DEFINE(common,
                get_pos_msg& set_pos_msg& move_msg& priority(1000, trace_msg) &
                        imgui_properties_msg);

class hierarchical : public hierarchical_gen
{
    HA_MESSAGES_IN_MIXIN(hierarchical)
public:
    eid                     get_parent() const { return parent; }
    const std::vector<eid>& get_children() const { return children; }

    void set_parent(eid _parent) {
        PPK_ASSERT(parent == eid::invalid());
        parent = _parent;
        //::add_child(ObjectManager::get().getObject(_parent), ha_this.id());
    }
    void add_child(eid child) {
        //PPK_ASSERT(::get_parent(ObjectManager::get().getObject(child)) == eid::invalid());
        PPK_ASSERT(std::find(children.begin(), children.end(), child) == children.end());
        children.push_back(child);
        //::set_parent(ObjectManager::get().getObject(child), ha_this.id());
    }
    void remove_child(eid child) {
        auto it = std::find(children.begin(), children.end(), child);
        PPK_ASSERT(it != children.end());
        children.erase(it);
    }
};

HA_MIXIN_DEFINE(hierarchical,
                get_parent_msg& get_children_msg& set_parent_msg& add_child_msg& remove_child_msg);
