#include "editor.h"

#include "core/Application.h"
#include "core/World.h"

HA_SUPPRESS_WARNINGS
#include <boost/range/adaptor/reversed.hpp>
#include <imgui/imgui_internal.h>
HA_SUPPRESS_WARNINGS_END

HA_CLANG_SUPPRESS_WARNING("-Wformat-security")

// determines if the value is in the range - where it isn't clear which of the 2 bounds is bigger
static bool is_in_range(int val, int bound_1, int bound_2) {
    int smaller = Utils::Min(bound_1, bound_2);
    int bigger  = Utils::Max(bound_1, bound_2);

    return val >= smaller && val <= bigger;
}

void editor::update_gui() {
    update_selected();

    ImGui::SetNextWindowSize(ImVec2(300, 550), ImGuiSetCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiSetCond_FirstUseEver);
    if(ImGui::Begin("scene explorer", nullptr, ImGuiWindowFlags_MenuBar)) {
        if(ImGui::BeginMenuBar()) {
            static bool new_object = false;
            if(ImGui::BeginMenu("Objects")) {
                ImGui::MenuItem("New", nullptr, &new_object);
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();

            if(new_object) {
                create_object();
                new_object = false;
            }
        }

        std::vector<oid> to_select;
        std::vector<oid> to_deselect;
        oid new_parent_for_selected; // will become the parent of middle-mouse-button-dragged selected objects

        static ImGuiTextFilter filter;
        filter.Draw("Filter (inc,-exc)", 150.f);

        if(ImGui::TreeNodeEx((const void*)"obs", ImGuiTreeNodeFlags_DefaultOpen, "objects")) {
            // recursive select/deselect
            std::function<void(oid, bool)> recursiveSelecter = [&](oid root, bool select) {
                auto& root_obj = root.obj();
                if(select && !root_obj.has<selected>())
                    to_select.push_back(root);
                else if(root_obj.has<selected>())
                    to_deselect.push_back(root);

                // recurse through children
                const auto& children = ::get_children(root_obj);
                for(const auto& c : children)
                    recursiveSelecter(c, select);
            };

            // recursive tree build
            std::function<void(oid, bool)> buildTree = [&](oid root, bool display) {
                auto&              obj = root.obj();
                ImGuiTreeNodeFlags node_flags =
                        ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
                        ImGuiTreeNodeFlags_DefaultOpen |
                        (obj.has<selected>() ? ImGuiTreeNodeFlags_Selected : 0);

                const auto& children = ::get_children(obj);
                if(children.empty()) // make the node a leaf node if no children
                    node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

                auto name = obj.name() + " (" + std::to_string(int16(obj.id())) + ")";

                bool is_open        = false;
                bool filter_passing = filter.PassFilter(name.c_str());

                // display the current node only if its parent is displayed (and there are no filters) or it has passed filtering
                if((filter_passing && display) || (filter_passing && filter.IsActive())) {
                    is_open = ImGui::TreeNodeEx((void*)(intptr_t)int16(root), node_flags,
                                                name.c_str());
                    display &= is_open; // update the display flag for the children

                    // logic for dragging selected objects with the middle mouse button onto unselected objects for reparenting
                    static bool middle_click_started_on_selected = false;
                    // if the current item is selected and just got middle clicked
                    if(ImGui::IsItemClicked(2) && obj.has<selected>())
                        middle_click_started_on_selected = true;
                    // if the current item is hovered, the middle mouse button just got released
                    // and we have recorded a middle click start onto a selected item
                    if(ImGui::IsItemHovered() && ImGui::IsMouseReleased(2) &&
                       middle_click_started_on_selected) {
                        middle_click_started_on_selected = false;
                        // if the current item is not selected
                        if(!obj.has<selected>())
                            new_parent_for_selected = root;
                    }

                    // logic for normal selection through the first mouse button
                    if(ImGui::IsItemClicked()) {
                        bool shouldSelect = !obj.has<selected>();

                        if(ImGui::GetIO().KeyShift) {
                            recursiveSelecter(root, shouldSelect);
                        } else if(ImGui::GetIO().KeyCtrl) {
                            if(shouldSelect)
                                to_select.push_back(root);
                            else
                                to_deselect.push_back(root);
                        } else if(shouldSelect) {
                            for(auto& it : m_selected)
                                to_deselect.push_back(it);
                            to_select.push_back(root);
                        }
                    }
                }

                // always recurse through children because they should be displayed when filtering even if their parent is closed
                if(!children.empty()) {
                    for(const auto& c : children)
                        buildTree(c, display);
                    if(is_open)
                        ImGui::TreePop();
                }
            };

            for(auto& curr : ObjectManager::get().getObjects()) {
                // recurse from those without a parent only
                if(curr.second.implements(get_const_parent_msg)) {
                    if(::get_parent(curr.second) == oid::invalid())
                        buildTree(curr.second.id(), true);
                }
            }

            ImGui::TreePop();
        }

        update_selection(to_select, to_deselect);
        reparent(new_parent_for_selected);
    }
    ImGui::End();

    ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiSetCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(float(Application::get().width() - 410), 10),
                            ImGuiSetCond_FirstUseEver);

    if(ImGui::Begin("object attributes", nullptr, ImGuiWindowFlags_MenuBar)) {
        if(ImGui::BeginMenuBar()) {
            static bool add_to_selected = false;
            static bool remove_selected = false;
            static bool remove_by_name  = false;
            if(ImGui::BeginMenu("Mixins")) {
                ImGui::MenuItem("Add to selected", nullptr, &add_to_selected);
                ImGui::MenuItem("Remove selected", nullptr, &remove_selected);
                ImGui::MenuItem("Remove by name", nullptr, &remove_by_name);
                if(m_selected.empty())
                    add_to_selected = false;
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();

            auto mixin_selector_modal = [&](cstr modal_name, bool& to_clear_on_close,
                                            std::vector<const mixin_type_info*>& filtered_mixins) {
                bool res = false;

                ImGui::OpenPopup(modal_name);
                if(ImGui::BeginPopupModal(modal_name, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                    static ImGuiTextFilter filter;
                    filter.Draw("Filter (inc,-exc)");
                    auto& all_mixins = getAllMixins();
                    for(auto& mixin : all_mixins)
                        if(filter.PassFilter(mixin.first.c_str()))
                            ImGui::BulletText("%s", mixin.first.c_str());

                    if(ImGui::Button("OK", ImVec2(120, 0))) {
                        for(auto& mixin : all_mixins)
                            if(filter.PassFilter(mixin.first.c_str()))
                                filtered_mixins.push_back(mixin.second.get_mixin_type_info());

                        res = true;
                        ImGui::CloseCurrentPopup();
                        to_clear_on_close = false;
                        filter.Clear();
                    }
                    ImGui::SameLine();
                    if(ImGui::Button("Cancel", ImVec2(120, 0))) {
                        ImGui::CloseCurrentPopup();
                        to_clear_on_close = false;
                        filter.Clear();
                    }
                    ImGui::EndPopup();
                }
                return res;
            };

            if(add_to_selected) {
                std::vector<const mixin_type_info*> filtered;
                if(mixin_selector_modal("Add mixins to selected", add_to_selected, filtered))
                    add_mixins_to_selected(filtered);
            }
            if(remove_selected) {
                remove_selected_mixins();
                remove_selected = false;
            }
            if(remove_by_name) {
                std::vector<const mixin_type_info*> filtered;
                if(mixin_selector_modal("Remove mixins from selected", remove_by_name, filtered))
                    remove_mixins_by_name_from_selected(filtered);
            }
        }

        static ImGuiTextFilter mixins_to_show_filter;
        mixins_to_show_filter.Draw("Filter by mixin");

        for(auto& id : m_selected) {
            auto& obj = id.obj();

            std::vector<cstr> mixin_names;
            obj.get_mixin_names(mixin_names);
            auto found = std::find_if(mixin_names.begin(), mixin_names.end(), [&](cstr in) {
                return mixins_to_show_filter.PassFilter(in);
            });
            if(found == mixin_names.end())
                continue;

            if(ImGui::TreeNodeEx((const void*)obj.name().c_str(), ImGuiTreeNodeFlags_DefaultOpen,
                                 obj.name().c_str())) {
                // attributes of the object itself
                imgui_bind_attributes(obj, "", obj);

                // attributes of the mixins - can select them
                imgui_binding_callbacks cbs;
                common::get_imgui_binding_callbacks_from_mixins(obj, cbs);
                auto& selected_mixin = *obj.get<selected>();
                for(auto& curr : cbs) {
                    if(!mixins_to_show_filter.PassFilter(curr.first->name))
                        continue;

                    auto curr_id           = curr.first->id;
                    bool is_mixin_selected = selected_mixin.selected_mixins.count(curr_id) != 0;

                    ImGuiTreeNodeFlags flags =
                            ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
                            (is_mixin_selected ? ImGuiTreeNodeFlags_Selected : 0);

                    bool node_open = ImGui::TreeNodeEx(curr.first->name, flags, curr.first->name);
                    if(ImGui::IsItemClicked()) {
                        if(!is_mixin_selected)
                            selected_mixin.selected_mixins.insert({curr_id, curr.first->name});
                        else if(ImGui::GetIO().KeyCtrl) // can deselect by holding Ctrl
                            selected_mixin.selected_mixins.erase(curr_id);
                    }
                    if(node_open) {
                        curr.second(obj);
                        ImGui::TreePop();
                    }
                }

                ImGui::TreePop();
            }
        }
    }
    ImGui::End();

    ImGui::SetNextWindowSize(ImVec2(300, 500), ImGuiSetCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(10, float(Application::get().height() - 510)),
                            ImGuiSetCond_FirstUseEver);

    if(ImGui::Begin("command history", nullptr, ImGuiWindowFlags_MenuBar)) {
        if(ImGui::BeginMenuBar()) {
            static bool merge_selected = false;
            if(ImGui::BeginMenu("Edit")) {
                ImGui::MenuItem("Merge selected", nullptr, &merge_selected);
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();

            if(merge_selected) {
                if(m_selected_command_idx_1 != -1) {
                    hassert(m_selected_command_idx_2 != -1);
                    merge_commands();
                }
                merge_selected = false;
            }
        }

        auto current_command_color = ImColor(1.0f, 0.2f, 0.2f);

        // recursive display of the command history
        std::function<void(const command_variant&, int, bool)> showCommands =
                [&](const command_variant& c, int curr_top_most, bool is_soft) {
                    using namespace std::string_literals; // for "s" suffix returning a std::string

                    bool is_compound = c.type() == boost::typeindex::type_id<compound_cmd>();
                    bool is_top_most = curr_top_most != -1;
                    bool is_curr     = is_top_most && curr_top_most == curr_undo_redo;

                    if(is_curr && m_should_rescroll_in_command_history) {
                        ImGui::SetScrollHere();
                        m_should_rescroll_in_command_history = false;
                    }

                    // if there are soft commands - display them above the current one
                    if(is_curr && soft_undo_redo_commands.size())
                        for(auto& curr_soft : boost::adaptors::reverse(soft_undo_redo_commands))
                            showCommands(curr_soft, -1, true);

                    char buff[256];
                    snprintf(buff, HA_COUNT_OF(buff), "%2d ", curr_top_most + 1);
                    std::string        name       = is_top_most ? buff : "   ";
                    std::string        desc       = "Description: ";
                    ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow;
                    if(!is_compound) // make the node a leaf node if not compound
                        node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

                    // determine if the current top-most command is selected (or in a selected range)
                    if(is_top_most && m_selected_command_idx_1 != -1) {
                        if(is_in_range(curr_top_most, m_selected_command_idx_1,
                                       m_selected_command_idx_2))
                            node_flags |= ImGuiTreeNodeFlags_Selected;
                    }

                    const JsonData* to_display   = nullptr;
                    const JsonData* to_display_2 = nullptr;
                    if(is_compound) {
                        const auto& cmd = boost::get<compound_cmd>(c);
                        name += "[composite] "s + cmd.description;
                    } else if(c.type() == boost::typeindex::type_id<attributes_changed_cmd>()) {
                        const auto& cmd = boost::get<attributes_changed_cmd>(c);
                        name += "[attribute] <"s + cmd.name + ">";
                        to_display   = &cmd.old_val;
                        to_display_2 = &cmd.new_val;
                        desc += cmd.description;
                    } else if(c.type() == boost::typeindex::type_id<object_creation_cmd>()) {
                        const auto& cmd = boost::get<object_creation_cmd>(c);
                        name += "[obj : "s + (cmd.created ? "new" : "del") + "] <"s + cmd.name +
                                ">";
                        to_display = &cmd.state;
                    } else if(c.type() == boost::typeindex::type_id<object_mutation_cmd>()) {
                        const auto& cmd = boost::get<object_mutation_cmd>(c);
                        name += "[mut : "s + (cmd.added ? "add" : "rem") + "] <" + cmd.name + ">";
                        to_display = &cmd.state;
                        // append to the description the list of mixins
                        for(auto& curr : cmd.mixins)
                            desc += "'"s + curr + "', ";
                        desc.pop_back();
                        desc.pop_back();
                    }

                    if(is_soft)
                        ImGui::PushStyleColor(ImGuiCol_Text, ImColor(0.2f, 0.2f, 1.0f));
                    if(is_curr)
                        ImGui::PushStyleColor(ImGuiCol_Text, current_command_color);
                    bool is_open = ImGui::TreeNodeEx((const void*)&c, node_flags, name.c_str());
                    if(is_curr)
                        ImGui::PopStyleColor(1);
                    if(is_soft)
                        ImGui::PopStyleColor(1);

                    // update selection range
                    if(is_top_most && ImGui::IsMouseClicked(0) && ImGui::IsItemHovered()) {
                        if(ImGui::GetIO().KeyShift) {
                            if(m_selected_command_idx_1 == -1) {
                                m_selected_command_idx_1 = curr_top_most;
                                m_selected_command_idx_2 = curr_top_most;
                            } else {
                                m_selected_command_idx_2 = curr_top_most;
                            }
                        } else {
                            if(m_selected_command_idx_1 == curr_top_most) {
                                m_selected_command_idx_1 = -1;
                                m_selected_command_idx_2 = -1;
                            } else {
                                m_selected_command_idx_1 = curr_top_most;
                                m_selected_command_idx_2 = curr_top_most;
                            }
                        }
                    }

                    if(is_top_most && !is_curr && ImGui::IsMouseDoubleClicked(0) &&
                       ImGui::IsItemHovered())
                        fast_forward_to_command(curr_top_most);

                    if(to_display &&
                       ImGui::BeginPopupContextItem(std::to_string((uintptr_t)&c).c_str())) {
                        if(to_display && ImGui::Button("Inspect"))
                            ImGui::OpenPopup("json contents");
                        if(ImGui::BeginPopupModal("json contents", nullptr,
                                                  ImGuiWindowFlags_AlwaysAutoResize)) {
                            ImGui::TextWrapped(desc.c_str());
                            ImGui::Spacing();
                            ImGui::Spacing();
                            ImGui::Separator();
                            if(to_display_2) {
                                ImGui::Text("old:");
                                ImGui::Separator();
                                ImGui::TextWrapped(to_display->data().data());
                                ImGui::Separator();
                                ImGui::Text("new:");
                                ImGui::Separator();
                                ImGui::TextWrapped(to_display_2->data().data());
                                if(ImGui::Button("Copy to clipboard")) {
                                    std::string appended(to_display->data().data());
                                    appended += "\n\n";
                                    appended += to_display_2->data().data();
                                    ImGui::SetClipboardText(appended.c_str());
                                }
                            } else {
                                ImGui::TextWrapped(to_display->data().data());
                                if(ImGui::Button("Copy to clipboard"))
                                    ImGui::SetClipboardText(to_display->data().data());
                            }
                            ImGui::SameLine();
                            if(ImGui::Button("Close"))
                                ImGui::CloseCurrentPopup();
                            ImGui::EndPopup();
                        }
                        ImGui::EndPopup();
                    }

                    if(is_open && is_compound) {
                        auto& comp_cmd = boost::get<compound_cmd>(c);
                        for(auto& part : comp_cmd.commands)
                            showCommands(part, -1, false);
                        ImGui::TreePop();
                    }

                };

        int curr_idx = int(undo_redo_commands.size()) - 1;
        for(auto& curr : boost::adaptors::reverse(undo_redo_commands))
            showCommands(curr, curr_idx--, false);

        // add a leaf node for the start of the history
        if(curr_undo_redo == -1)
            ImGui::PushStyleColor(ImGuiCol_Text, current_command_color);
        ImGui::TreeNodeEx("", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen,
                          " 0 [start of history]");
        if(curr_undo_redo == -1)
            ImGui::PopStyleColor(1);

        if(curr_undo_redo != -1 && ImGui::IsMouseDoubleClicked(0) && ImGui::IsItemHovered())
            fast_forward_to_command(-1);
    }
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(320, 10), ImGuiSetCond_FirstUseEver);
    ImGui::ShowTestWindow();
}

void editor::update_gizmo() {
    // don't continue if no gizmo-able objects are selected
    if(selected_with_gizmo.empty())
        return;

    auto& app = Application::get();

    // gizmo context/state update
    m_gizmo_state.viewport_size     = {float(app.width()), float(app.height())};
    m_gizmo_state.cam.near_clip     = 0.1f;
    m_gizmo_state.cam.far_clip      = 1000.f;
    m_gizmo_state.cam.yfov          = yama::deg_to_rad(45.f);
    m_gizmo_state.screenspace_scale = 80.f; // 80px screenspace - or something like that
    auto cam_pos                    = tr::get_pos(World::get().camera().obj());
    auto cam_rot                    = tr::get_rot(World::get().camera().obj());
    m_gizmo_state.cam.position      = {cam_pos.x, cam_pos.y, cam_pos.z};
    m_gizmo_state.cam.orientation   = {cam_rot.x, cam_rot.y, cam_rot.z, cam_rot.w};
    m_gizmo_ctx.update(m_gizmo_state);

    // update gizmo position to be between all selected objects
    if(!mouse_button_left_changed && !m_gizmo_state.mouse_left) {
        yama::vector3    avg_pos = {0, 0, 0};
        yama::quaternion avg_rot =
                (selected_with_gizmo.size() == 1) ?                 // based on number of objects
                        tr::get_rot(selected_with_gizmo[0].obj()) : // orientation of object
                        yama::quaternion::identity();               // generic default rotation
        for(auto& curr : selected_with_gizmo)
            avg_pos += tr::get_pos(curr.obj());
        avg_pos /= float(selected_with_gizmo.size());

        gizmo_transform.position    = {avg_pos.x, avg_pos.y, avg_pos.z};
        gizmo_transform.orientation = {avg_rot.x, avg_rot.y, avg_rot.z, avg_rot.w};
        gizmo_transform.scale       = {1, 1, 1}; // no need for anything different

        gizmo_transform_last = gizmo_transform;
    }

    // record gizmo transform on start of usage (+transforms of selected objects)
    if(mouse_button_left_changed) {
        if(m_gizmo_state.mouse_left) {
            gizmo_transform_last = gizmo_transform;
            for(auto& id : selected_with_gizmo) {
                id.obj().get<selected>()->old_t       = tr::get_transform(id.obj());
                id.obj().get<selected>()->old_local_t = tr::get_transform_local(id.obj());
            }
        }
    }

    // update gizmo
    tinygizmo::transform_gizmo("gizmo", m_gizmo_ctx, gizmo_transform);

    // if (probably) using gizmo - need this to really determine it: https://github.com/ddiakopoulos/tinygizmo/issues/6
    if(m_gizmo_state.mouse_left) {
        auto pos   = yama::vector3::from_ptr(&gizmo_transform.position[0]);
        auto pos_l = yama::vector3::from_ptr(&gizmo_transform_last.position[0]);
        auto scl   = yama::vector3::from_ptr(&gizmo_transform.scale[0]);
        auto scl_l = yama::vector3::from_ptr(&gizmo_transform_last.scale[0]);
        auto rot   = yama::quaternion::from_ptr(&gizmo_transform.orientation[0]);
        auto rot_l = yama::quaternion::from_ptr(&gizmo_transform_last.orientation[0]);
        if(!yama::close(pos, pos_l) || !yama::close(scl, scl_l) || !yama::close(rot, rot_l)) {
            for(auto& id : selected_with_gizmo) {
                auto t = id.obj().get<selected>()->old_t;
                t.pos += pos - pos_l;
                t.scl += scl - scl_l;
                if(selected_with_gizmo.size() == 1) {
                    t.rot = rot; // the gizmo is attached to the object's orientation so this is a straight copy
                } else {
                    // can change the order - does something else but is still ok and sort-of logical
                    //t.rot = t.rot * rot; // local space
                    t.rot = rot * t.rot; // world space
                }
                tr::set_transform(id.obj(), t);
            }
        }
    }

    // check if anything changed after release
    if(mouse_button_left_changed) {
        if(!m_gizmo_state.mouse_left) {
            handle_gizmo_changes();
        }
    }
    mouse_button_left_changed = false;

    m_gizmo_ctx.draw();
}

HA_CLANG_SUPPRESS_WARNING_END
