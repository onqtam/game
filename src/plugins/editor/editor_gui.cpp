#include "editor.h"

#include "core/Application.h"
#include "core/World.h"

HA_SUPPRESS_WARNINGS
#include <imgui/imgui_internal.h>
HA_SUPPRESS_WARNINGS_END

void editor::update_gui() {
    static bool no_titlebar  = false;
    static bool no_border    = true;
    static bool no_resize    = false;
    static bool no_move      = false;
    static bool no_scrollbar = false;
    static bool no_collapse  = false;
    static bool no_menu      = true;

    // Demonstrate the various window flags. Typically you would just use the default.
    ImGuiWindowFlags window_flags = 0;
    // clang-format off
    if (no_titlebar)  window_flags |= ImGuiWindowFlags_NoTitleBar;
    if (!no_border)   window_flags |= ImGuiWindowFlags_ShowBorders;
    if (no_resize)    window_flags |= ImGuiWindowFlags_NoResize;
    if (no_move)      window_flags |= ImGuiWindowFlags_NoMove;
    if (no_scrollbar) window_flags |= ImGuiWindowFlags_NoScrollbar;
    if (no_collapse)  window_flags |= ImGuiWindowFlags_NoCollapse;
    if (!no_menu)     window_flags |= ImGuiWindowFlags_MenuBar;
    ImGui::SetNextWindowSize(ImVec2(300,600), ImGuiSetCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiSetCond_FirstUseEver);
    // clang-format on

    update_selected();

    if(ImGui::Begin("scene explorer", nullptr, window_flags)) {
        std::vector<oid> to_select;
        std::vector<oid> to_deselect;
        oid new_parent_for_selected; // will become the parent of middle-mouse-button-dragged selected objects

        if(ImGui::TreeNodeEx((const void*)"obs", ImGuiTreeNodeFlags_DefaultOpen, "objects")) {
            static ImGuiTextFilter filter;
            filter.Draw("Filter (inc,-exc)", 150.f);

            // recursive select/deselect
            std::function<void(oid, bool)> recursiveSelecter = [&](oid root, bool select) {
                auto& root_obj = root.obj();
                if(select && !root_obj.has(selected_mixin_id))
                    to_select.push_back(root);
                else if(root_obj.has(selected_mixin_id))
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
                        (obj.has(selected_mixin_id) ? ImGuiTreeNodeFlags_Selected : 0);

                const auto& children = ::get_children(obj);
                if(children.empty()) // make the node a leaf node if no children
                    node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

                auto name = obj.name() + " (" + std::to_string(int16(obj.id())) + ")";

                bool is_open        = false;
                bool filter_passing = filter.PassFilter(name.c_str());

                // display the current node only if its parent is displayed (and there are no filters) or it has passed filtering
                if((filter_passing && display) || (filter_passing && filter.IsActive())) {
                    HA_CLANG_SUPPRESS_WARNING("-Wformat-security")
                    is_open = ImGui::TreeNodeEx((void*)(intptr_t)int16(root), node_flags,
                                                name.c_str());
                    HA_CLANG_SUPPRESS_WARNING_END
                    display &= is_open; // update the display flag for the children

                    // logic for dragging selected objects with the middle mouse button onto unselected objects for reparenting
                    static bool middle_click_started_on_selected = false;
                    // if the current item is selected and just got middle clicked
                    if(ImGui::IsItemClicked(2) && obj.has(selected_mixin_id))
                        middle_click_started_on_selected = true;
                    // if the current item is hovered, the middle mouse button just got released
                    // and we have recorded a middle click start onto a selected item
                    if(ImGui::IsItemHovered() && ImGui::IsMouseReleased(2) &&
                       middle_click_started_on_selected) {
                        middle_click_started_on_selected = false;
                        // if the current item is not selected
                        if(!obj.has(selected_mixin_id))
                            new_parent_for_selected = root;
                    }

                    // logic for normal selection through the first mouse button
                    if(ImGui::IsItemClicked()) {
                        bool shouldSelect = !obj.has(selected_mixin_id);

                        if(ImGui::GetIO().KeyShift) {
                            recursiveSelecter(root, shouldSelect);
                        } else if(ImGui::GetIO().KeyCtrl) {
                            if(shouldSelect)
                                to_select.push_back(root);
                            else
                                to_deselect.push_back(root);
                        } else if(shouldSelect) {
                            for(auto& it : selected)
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

    if(ImGui::Begin("object attributes", nullptr, window_flags)) {
        for(auto& id : selected) {
            auto& obj = id.obj();
            HA_CLANG_SUPPRESS_WARNING("-Wformat-security")
            if(ImGui::TreeNodeEx((const void*)obj.name().c_str(), ImGuiTreeNodeFlags_DefaultOpen,
                                 obj.name().c_str())) {
                HA_CLANG_SUPPRESS_WARNING_END
                // attributes of the object itself
                imgui_bind_attributes(obj, "", obj);
                // attributes of the mixins
                if(obj.implements(common::imgui_bind_attributes_mixins_msg))
                    common::imgui_bind_attributes_mixins(obj);

                ImGui::TreePop();
            }
        }
    }
    ImGui::End();

    //ImGui::ShowTestWindow();
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
    }

    // record gizmo transform on start of usage (+transforms of selected objects)
    if(mouse_button_left_changed) {
        if(m_gizmo_state.mouse_left) {
            gizmo_transform_last = gizmo_transform;
            for(auto& id : selected_with_gizmo) {
                sel::get_transform_on_gizmo_start(id.obj()) = tr::get_transform(id.obj());
                sel::get_transform_local_on_gizmo_start(id.obj()) =
                        tr::get_transform_local(id.obj());
            }
        }
    }

    // update gizmo
    tinygizmo::transform_gizmo("gizmo", m_gizmo_ctx, gizmo_transform);

    // if (probably) using gizmo - need this to really determine it: https://github.com/ddiakopoulos/tinygizmo/issues/6
    if(m_gizmo_state.mouse_left) {
        auto diff_pos = gizmo_transform.position - gizmo_transform_last.position;
        auto diff_scl = gizmo_transform.scale - gizmo_transform_last.scale;
        auto rot      = (yama::quaternion&)gizmo_transform.orientation;

        // always update transforms - cannot figure out how to check for the rotation - I suck at math :(
        //if(minalg::length2(diff_pos) > 0 || minalg::length2(diff_scl) > 0 || glm::length(rot) != 1)
        {
            for(auto& id : selected_with_gizmo) {
                auto t = sel::get_transform_on_gizmo_start(id.obj());
                t.pos += yama::v(diff_pos.x, diff_pos.y, diff_pos.z);
                t.scl += yama::v(diff_scl.x, diff_scl.y, diff_scl.z);
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
