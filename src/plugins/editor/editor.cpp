#include "core/registry/registry.h"
#include "core/serialization/serialization.h"
#include "core/serialization/serialization_2.h"
#include "core/imgui/imgui_stuff.h"

#include "core/messages/messages.h"
#include "core/messages/messages_editor.h"
#include "core/Application.h"
#include "core/World.h"
#include "core/GraphicsHelpers.h"
#include "core/InputEvent.h"

HA_SUPPRESS_WARNINGS
#include <boost/range/adaptor/reversed.hpp>

#include <imgui/imgui_internal.h>

#include <GLFW/glfw3.h>
HA_SUPPRESS_WARNINGS_END

HA_GCC_SUPPRESS_WARNING("-Wzero-as-null-pointer-constant") // because of boost::variant's ctor

struct attributes_changed_cmd
{
    HA_FRIENDS_OF_TYPE(attributes_changed_cmd);

public:
    FIELD oid e;
    FIELD json_buf old_val;
    FIELD json_buf new_val;
};

struct object_creation_cmd
{
    HA_FRIENDS_OF_TYPE(object_creation_cmd);

public:
    FIELD oid id;
    FIELD json_buf object_state;
    FIELD bool     created;
};

struct object_mutation_cmd
{
    HA_FRIENDS_OF_TYPE(object_mutation_cmd);

public:
    FIELD oid id;
    FIELD std::vector<std::string> mixins;
    FIELD json_buf mixins_state;
    FIELD bool     added;
};

struct compound_cmd
{
    HA_FRIENDS_OF_TYPE(compound_cmd);

public:
    typedef boost::variant<attributes_changed_cmd, object_mutation_cmd, object_creation_cmd,
                           compound_cmd>
                                         command_variant;
    typedef std::vector<command_variant> commands_vector;

    FIELD commands_vector commands;
};

typedef compound_cmd::command_variant command_variant;
typedef compound_cmd::commands_vector commands_vector;

class editor : public UpdatableMixin<editor>, public InputEventListener, public Singleton<editor>
{
    HA_SINGLETON(editor);
    HA_MESSAGES_IN_MIXIN(editor);

    FIELD std::vector<oid> selected;
    FIELD std::vector<oid> selected_with_gizmo;
    FIELD commands_vector undo_redo_commands;
    FIELD int             curr_undo_redo            = -1;
    FIELD bool            mouse_button_left_changed = false;
    FIELD tinygizmo::rigid_transform gizmo_transform;
    FIELD tinygizmo::rigid_transform gizmo_transform_last;

    // these members are OK to not be serialized because they are constantly updated
    tinygizmo::gizmo_application_state m_gizmo_state;
    tinygizmo::gizmo_context           m_gizmo_ctx;
    std::vector<char>                  m_verts;
    std::vector<char>                  m_inds;
    ShaderHandle                       m_program;
    bgfx_vertex_decl                   vd;
    bgfx_vertex_buffer_handle          m_vert_buf = {BGFX_INVALID_HANDLE};
    bgfx_index_buffer_handle           m_ind_buf  = {BGFX_INVALID_HANDLE};

    dynamix::mixin_id selected_mixin_id =
            dynamix::internal::domain::instance().get_mixin_id_by_name("selected");

    GeomHandle   m_grid;
    ShaderHandle m_grid_shader;

    // until the allocator model of dynamix is extended we shall update this list manually like this
    void updateSelected() {
        selected.clear();
        selected_with_gizmo.clear();
        for(auto& curr : ObjectManager::get().getObjects()) {
            if(curr.second.has(selected_mixin_id)) {
                selected.push_back(curr.second.id());
                if(curr.second.implements(sel::no_gizmo_msg))
                    continue;
                selected_with_gizmo.push_back(curr.second.id());
            }
        }
    }

public:
    editor()
            : Singleton(this) {
        m_grid        = GeomMan::get().get("", createGrid, 20, 20, World::get().width(),
                                    World::get().height(), colors::green);
        m_grid_shader = ShaderMan::get().get("cubes");

        m_program = ShaderMan::get().get("gizmo");
        bgfx_vertex_decl_begin(&vd, BGFX_RENDERER_TYPE_COUNT);
        bgfx_vertex_decl_add(&vd, BGFX_ATTRIB_POSITION, 3, BGFX_ATTRIB_TYPE_FLOAT, false, false);
        bgfx_vertex_decl_add(&vd, BGFX_ATTRIB_NORMAL, 3, BGFX_ATTRIB_TYPE_FLOAT, false, false);
        bgfx_vertex_decl_add(&vd, BGFX_ATTRIB_COLOR0, 4, BGFX_ATTRIB_TYPE_FLOAT, false, false);
        bgfx_vertex_decl_end(&vd);

        m_gizmo_ctx.render = [&](const tinygizmo::geometry_mesh& r) {
            auto identity = yama::matrix::identity();
            bgfx_set_transform((float*)&identity, 1);

            m_verts.resize(r.vertices.size() * sizeof(tinygizmo::geometry_vertex));
            memcpy(m_verts.data(), r.vertices.data(), m_verts.size());
            m_inds.resize(r.triangles.size() * sizeof(r.triangles[0]));
            memcpy(m_inds.data(), r.triangles.data(), m_inds.size());

            // TODO: fix this! use dynamic or transient buffers (whatever that means)
            // and just update them instead of constantly recreating them
            if(m_vert_buf.idx != BGFX_INVALID_HANDLE) {
                bgfx_destroy_vertex_buffer(m_vert_buf);
                bgfx_destroy_index_buffer(m_ind_buf);
            }
            m_vert_buf = bgfx_create_vertex_buffer(
                    bgfx_make_ref(m_verts.data(), uint32(m_verts.size())), &vd, BGFX_BUFFER_NONE);
            m_ind_buf = bgfx_create_index_buffer(
                    bgfx_make_ref(m_inds.data(), uint32(m_inds.size())), BGFX_BUFFER_INDEX32);

            bgfx_set_vertex_buffer(0, m_vert_buf, 0, UINT32_MAX);
            bgfx_set_index_buffer(m_ind_buf, 0, UINT32_MAX);
            bgfx_set_state(BGFX_STATE_DEFAULT, 0);
            bgfx_submit(1, m_program.get(), 0, false);
        };
    }

    ~editor() {
        if(m_vert_buf.idx != BGFX_INVALID_HANDLE) {
            bgfx_destroy_vertex_buffer(m_vert_buf);
            bgfx_destroy_index_buffer(m_ind_buf);
        }
    }

    void update(float) {
        // draw grid
        auto identity = yama::matrix::identity();
        bgfx_set_transform((float*)&identity, 1);
        bgfx_set_vertex_buffer(0, m_grid.get().vbh, 0, UINT32_MAX);
        bgfx_set_state(BGFX_STATE_DEFAULT | m_grid.get().state, 0);
        bgfx_submit(0, m_grid_shader.get(), 0, false);

        auto& app = Application::get();
        auto& om  = ObjectManager::get();

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

        updateSelected();

        if(ImGui::Begin("scene explorer", nullptr, window_flags)) {
            std::vector<oid> to_select;
            std::vector<oid> to_deselect;

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

                for(auto& curr : om.getObjects()) {
                    // recurse from those without a parent only
                    if(curr.second.implements(get_const_parent_msg)) {
                        if(::get_parent(curr.second) == oid::invalid())
                            buildTree(curr.second.id(), true);
                    }
                }
                ImGui::TreePop();
            }

            // if the selection has changed - construct a compound command for the selected mixin and the affected objects
            if(to_select.size() + to_deselect.size() > 0) {
                compound_cmd comp_cmd;
                comp_cmd.commands.reserve(to_select.size() + to_deselect.size());

                auto add_mutate_command = [&](oid id, bool select) {
                    JsonData state = mixin_state(id.obj(), "selected");
                    comp_cmd.commands.push_back(
                            object_mutation_cmd({id, {"selected"}, state.data(), select}));
                };

                for(auto curr : to_select) {
                    add_mutate_command(curr, true);
                    curr.obj().addMixin("selected");
                }
                for(auto curr : to_deselect) {
                    add_mutate_command(curr, false);
                    curr.obj().remMixin("selected");
                }

                add_command(comp_cmd);

                //re-update the list for later usage
                updateSelected();
            }
        }
        ImGui::End();

        ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiSetCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(float(app.width() - 410), 10), ImGuiSetCond_FirstUseEver);

        if(ImGui::Begin("object attributes", nullptr, window_flags)) {
            for(auto& id : selected) {
                auto& obj = id.obj();
                HA_CLANG_SUPPRESS_WARNING("-Wformat-security")
                if(ImGui::TreeNodeEx((const void*)obj.name().c_str(),
                                     ImGuiTreeNodeFlags_DefaultOpen, obj.name().c_str())) {
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

        updateGizmo();
    }

    void updateGizmo() {
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
                    (selected_with_gizmo.size() == 1) ? // based on number of objects
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

    void handle_gizmo_changes() {
        compound_cmd comp_cmd;

        for(auto& id : selected_with_gizmo) {
            auto old_t = sel::get_transform_local_on_gizmo_start(id.obj());
            auto new_t = tr::get_transform_local(id.obj());
            if(old_t.pos != new_t.pos) {
                JsonData ov = mixin_attr_state("tform", "pos", old_t.pos);
                JsonData nv = mixin_attr_state("tform", "pos", new_t.pos);
                comp_cmd.commands.push_back(attributes_changed_cmd({id, ov.data(), nv.data()}));
            }
            if(old_t.scl != new_t.scl) {
                JsonData ov = mixin_attr_state("tform", "scl", old_t.scl);
                JsonData nv = mixin_attr_state("tform", "scl", new_t.scl);
                comp_cmd.commands.push_back(attributes_changed_cmd({id, ov.data(), nv.data()}));
            }
            if(old_t.rot != new_t.rot) {
                JsonData ov = mixin_attr_state("tform", "rot", old_t.rot);
                JsonData nv = mixin_attr_state("tform", "rot", new_t.rot);
                comp_cmd.commands.push_back(attributes_changed_cmd({id, ov.data(), nv.data()}));
            }
            // update this - even though we havent started using the gizmo - or else this might break when deleting the object
            sel::get_transform_on_gizmo_start(id.obj())       = tr::get_transform(id.obj());
            sel::get_transform_local_on_gizmo_start(id.obj()) = tr::get_transform_local(id.obj());
        }
        if(!comp_cmd.commands.empty())
            add_command(comp_cmd);
    }

    void process_event(const InputEvent& ev) {
        if(ev.type == InputEvent::MOTION) {
            m_gizmo_state.cursor = {float(ev.motion.x), float(ev.motion.y)};
        } else if(ev.type == InputEvent::KEY) {
            auto key    = ev.key.key;
            auto action = ev.key.action;
            auto mods   = ev.key.mods;
            if(key == GLFW_KEY_LEFT_CONTROL)
                m_gizmo_state.hotkey_ctrl = (action != GLFW_RELEASE);
            if(key == GLFW_KEY_L)
                m_gizmo_state.hotkey_local = (action != GLFW_RELEASE);
            if(key == GLFW_KEY_T)
                m_gizmo_state.hotkey_translate = (action != GLFW_RELEASE);
            if(key == GLFW_KEY_R)
                m_gizmo_state.hotkey_rotate = (action != GLFW_RELEASE);
            if(key == GLFW_KEY_S)
                m_gizmo_state.hotkey_scale = (action != GLFW_RELEASE);

            // undo - with repeat
            if(key == GLFW_KEY_Z && (mods & GLFW_MOD_CONTROL) && (action != GLFW_RELEASE)) {
                if(curr_undo_redo >= 0) {
                    printf("[UNDO] current action in undo/redo stack: %d (a total of %d actions)\n",
                           curr_undo_redo - 1, int(undo_redo_commands.size()));
                    handle_command(undo_redo_commands[curr_undo_redo--], true);
                }
            }

            // redo - with repeat
            if(key == GLFW_KEY_Y && (mods & GLFW_MOD_CONTROL) && (action != GLFW_RELEASE)) {
                if(curr_undo_redo + 1 < int(undo_redo_commands.size())) {
                    printf("[REDO] current action in undo/redo stack: %d (a total of %d actions)\n",
                           curr_undo_redo + 1, int(undo_redo_commands.size()));
                    handle_command(undo_redo_commands[++curr_undo_redo], false);
                }
            }

            // group
            if(key == GLFW_KEY_G && (mods & GLFW_MOD_CONTROL) && (action == GLFW_PRESS)) {
                if(!selected.empty()) {
                    printf("[GROUP]\n");
                    compound_cmd comp_cmd;

                    auto find_lowest_common_ancestor = [&]() {
                        // go upwards from each selected node and update the visited count for each node
                        std::map<oid, int> visited_counts;
                        for(auto curr : selected) {
                            while(curr != oid::invalid()) {
                                visited_counts[curr]++;
                                curr = get_parent(curr.obj());
                            }
                        }

                        // remove any node that has been visited less times than the number of selected objects
                        Utils::erase_if(visited_counts,
                                        [&](auto in) { return in.second < int(selected.size()); });

                        // if there is a common ancestor - it will have the same visited count as the number of selected objects
                        if(visited_counts.size() == 1 &&
                           std::find(selected.begin(), selected.end(),
                                     visited_counts.begin()->first) == selected.end()) {
                            // if only one object is left after the filtering (common ancestor to all) and is not part of the selection
                            return visited_counts.begin()->first;
                        } else if(visited_counts.size() > 1) {
                            // if atleast 2 nodes have the same visited count - means that one of the selected nodes
                            // is also a common ancestor (also to itself) - we need to find it and get its parent
                            for(auto& curr : visited_counts)
                                if(curr.first.obj().has(selected_mixin_id))
                                    return get_parent(curr.first.obj());
                        }
                        // all other cases
                        return oid::invalid();
                    };

                    // create new group object
                    auto& group = ObjectManager::get().create("group");

                    // if there is a common ancestor - add the new group object as its child
                    auto common_ancestor = find_lowest_common_ancestor();
                    if(common_ancestor.isValid()) {
                        JsonData ancestor_old = mixin_state(common_ancestor.obj(), "parental");
                        set_parent(group, common_ancestor);
                        JsonData ancestor_new = mixin_state(common_ancestor.obj(), "parental");
                        comp_cmd.commands.push_back(attributes_changed_cmd(
                                {common_ancestor, ancestor_old.data(), ancestor_new.data()}));
                    }

                    // average position for the new group object
                    auto average_pos = yama::vector3::zero();

                    // save the transforms of the selected objects before changing parental information
                    std::vector<std::pair<oid, std::pair<transform, JsonData>>> old_transforms;

                    // mutate all the currently selected objects and deselect them
                    for(auto& curr : selected) {
                        // accumulate the position
                        average_pos += tr::get_pos(curr.obj());
                        // record the old transform
                        old_transforms.push_back({curr,
                                                  {tr::get_transform(curr.obj()),
                                                   mixin_state(curr.obj(), "tform")}});

                        // parent old state
                        auto     parent = get_parent(curr.obj());
                        JsonData parent_old;
                        if(parent.isValid())
                            parent_old = mixin_state(parent.obj(), "parental");

                        // record parental state of current object before change
                        JsonData curr_old = mixin_state(curr.obj(), "parental");

                        // set new parental relationship
                        set_parent(curr.obj(), group.id());

                        // parent new state & command submit
                        if(parent.isValid()) {
                            JsonData parent_new = mixin_state(parent.obj(), "parental");
                            comp_cmd.commands.push_back(attributes_changed_cmd(
                                    {parent, parent_old.data(), parent_new.data()}));
                        }

                        // current new state & command submit
                        JsonData curr_new = mixin_state(curr.obj(), "parental");
                        comp_cmd.commands.push_back(
                                attributes_changed_cmd({curr, curr_old.data(), curr_new.data()}));

                        // serialize the state of the mixins
                        JsonData selected_state = mixin_state(curr.obj(), "selected");
                        comp_cmd.commands.push_back(object_mutation_cmd(
                                {curr, {"selected"}, selected_state.data(), false}));

                        // remove the selection
                        curr.obj().remMixin("selected");
                    }

                    // set position of newly created group to be the average position of all selected objects
                    average_pos /= float(selected.size());
                    tr::set_transform(group, {average_pos, {1, 1, 1}, {0, 0, 0, 1}});

                    // fix the transforms after the position of the group has been set
                    for(auto& curr : old_transforms) {
                        // set the old world transform (will recalculate the local transform of the object)
                        tr::set_transform(curr.first.obj(), curr.second.first);
                        // add the changed transform to the undo/redo command list
                        JsonData new_tform = mixin_state(curr.first.obj(), "tform");
                        comp_cmd.commands.push_back(attributes_changed_cmd(
                                {curr.first, curr.second.second.data(), new_tform.data()}));
                    }

                    // select the new group object
                    group.addMixin("selected");

                    // add the created group object
                    JsonData state = object_state(group);
                    comp_cmd.commands.push_back(
                            object_creation_cmd({group.id(), state.data(), true}));
                    JsonData group_state = mixin_state(group, nullptr);
                    comp_cmd.commands.push_back(object_mutation_cmd(
                            {group.id(), mixin_names(group), group_state.data(), true}));

                    // add the compound command
                    add_command(comp_cmd);
                }
            }

            // duplicate
            if(key == GLFW_KEY_D && (mods & GLFW_MOD_CONTROL) && (action == GLFW_PRESS)) {
                if(!selected.empty()) {
                    printf("[DUPLICATE]\n");
                    compound_cmd comp_cmd;

                    // create new group object
                    auto& group = ObjectManager::get().create();

                    // make copies of all selected objects
                    for(auto& curr : selected) {
                        // make the copy and add it as a child to the new group
                        auto& copy = ObjectManager::get().create();
                        copy.copy_from(curr.obj());
                        set_parent(copy, group.id());

                        // add commands for its creation
                        JsonData state = object_state(copy);
                        comp_cmd.commands.push_back(
                                object_creation_cmd({copy.id(), state.data(), true}));
                        JsonData mix_state = mixin_state(copy, nullptr);
                        comp_cmd.commands.push_back(object_mutation_cmd(
                                {copy.id(), mixin_names(copy), mix_state.data(), true}));

                        // serialize the state of the currently selected mixins before unselecting them
                        JsonData selected_state = mixin_state(curr.obj(), "selected");
                        comp_cmd.commands.push_back(object_mutation_cmd(
                                {curr, {"selected"}, selected_state.data(), false}));

                        // remove the selection
                        curr.obj().remMixin("selected");
                    }

                    // select the new group object
                    group.addMixin("selected");

                    // add the created group object
                    JsonData state = object_state(group);
                    comp_cmd.commands.push_back(
                            object_creation_cmd({group.id(), state.data(), true}));
                    JsonData group_state = mixin_state(group, nullptr);
                    comp_cmd.commands.push_back(object_mutation_cmd(
                            {group.id(), mixin_names(group), group_state.data(), true}));

                    // add the compound command
                    add_command(comp_cmd);
                }
            }

            // delete selected objects
            if(key == GLFW_KEY_DELETE && (action != GLFW_RELEASE)) {
                if(!selected.empty()) {
                    printf("[DELETE]\n");
                    handle_gizmo_changes();

                    compound_cmd comp_cmd;
                    for(auto& curr : selected) {
                        auto detele_object = [&](Object& obj) {
                            // serialize the state of the mixins
                            JsonData mix_state = mixin_state(obj, nullptr);
                            comp_cmd.commands.push_back(object_mutation_cmd(
                                    {obj.id(), mixin_names(obj), mix_state.data(), false}));

                            // serialize the state of the object itself
                            JsonData state = object_state(obj);
                            comp_cmd.commands.push_back(
                                    object_creation_cmd({obj.id(), state.data(), false}));

                            ObjectManager::get().destroy(obj.id());
                        };

                        std::function<void(oid)> delete_unselected_children = [&](oid root) {
                            auto& root_obj = root.obj();
                            // recurse through children
                            const auto& children = ::get_children(root_obj);
                            for(const auto& c : children)
                                delete_unselected_children(c);
                            // delete only if not selected because we are iterating through the selected objects anyway
                            if(!root_obj.has(selected_mixin_id))
                                detele_object(root_obj);
                        };

                        delete_unselected_children(curr);
                        detele_object(curr.obj());
                    }
                    add_command(comp_cmd);

                    selected.clear();
                }
            }
        } else if(ev.type == InputEvent::BUTTON) {
            if(ev.button.button == 0)
                m_gizmo_state.mouse_left = (ev.button.action != GLFW_RELEASE);
            mouse_button_left_changed = true;
        }
    }

    void handle_command(command_variant& command_var, bool undo) {
        if(command_var.type() == boost::typeindex::type_id<attributes_changed_cmd>()) {
            auto&       cmd = boost::get<attributes_changed_cmd>(command_var);
            auto&       val = undo ? cmd.old_val : cmd.new_val;
            const auto& doc = JsonData::parse(val);
            hassert(doc.is_valid());
            const auto root = doc.get_root();
            hassert(root.get_length() == 1);
            if(strcmp(root.get_object_key(0).data(), "") == 0)
                deserialize(cmd.e.obj(), root.get_object_value(0)); // object attribute
            else
                common::deserialize_mixins(cmd.e.obj(), root); // mixin attribute
        } else if(command_var.type() == boost::typeindex::type_id<object_mutation_cmd>()) {
            auto& cmd = boost::get<object_mutation_cmd>(command_var);
            if((!cmd.added && undo) || (cmd.added && !undo)) {
                // add the mixins
                for(auto& mixin : cmd.mixins)
                    cmd.id.obj().addMixin(mixin.c_str());
                // deserialize the mixins
                const auto& doc = JsonData::parse(cmd.mixins_state);
                hassert(doc.is_valid());
                common::deserialize_mixins(cmd.id.obj(), doc.get_root());
            } else {
                // remove the mixins
                for(auto& mixin : cmd.mixins)
                    cmd.id.obj().remMixin(mixin.c_str());
            }
        } else if(command_var.type() == boost::typeindex::type_id<object_creation_cmd>()) {
            auto& cmd = boost::get<object_creation_cmd>(command_var);
            if((cmd.created && undo) || (!cmd.created && !undo)) {
                ObjectManager::get().destroy(cmd.id);
            } else {
                ObjectManager::get().createFromId(cmd.id);
                const auto& doc = JsonData::parse(cmd.object_state);
                hassert(doc.is_valid());
                deserialize(cmd.id.obj(), doc.get_root().get_object_value(0)); // object attributes
            }
        } else if(command_var.type() == boost::typeindex::type_id<compound_cmd>()) {
            auto& cmd = boost::get<compound_cmd>(command_var);
            if(undo)
                for(auto& curr : boost::adaptors::reverse(cmd.commands))
                    handle_command(curr, undo);
            else
                for(auto& curr : cmd.commands)
                    handle_command(curr, undo);
        }
    }

    void add_command(const command_variant& command) {
        if(!undo_redo_commands.empty())
            undo_redo_commands.erase(undo_redo_commands.begin() + 1 + curr_undo_redo,
                                     undo_redo_commands.end());

        undo_redo_commands.push_back(command);

        ++curr_undo_redo;
        printf("num actions in undo/redo stack: %d\n", curr_undo_redo);
    }

    void add_changed_attribute(oid e, const json_buf& old_val, const json_buf& new_val) {
        add_command(attributes_changed_cmd({e, old_val, new_val}));
    }
};

HA_GCC_SUPPRESS_WARNING_END

HA_SINGLETON_INSTANCE(editor);

HA_MIXIN_DEFINE(editor, Interface_editor);

#include <gen/editor.cpp.inl>
