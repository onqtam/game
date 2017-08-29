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

struct attribute_changed_cmd
{
    HA_FRIENDS_OF_TYPE(attribute_changed_cmd);

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
    typedef boost::variant<attribute_changed_cmd, object_mutation_cmd, object_creation_cmd,
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
        for(const auto& curr : ObjectManager::get().getObjects()) {
            if(curr.second.has(selected_mixin_id)) {
                selected.push_back(curr.second);
                if(curr.second.implements(sel::no_gizmo_msg))
                    continue;
                selected_with_gizmo.push_back(curr.second);
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
            auto identity = glm::mat4(1.f);
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
            bgfx_submit(0, m_program.get(), 0, false);
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
        auto identity = glm::mat4(1.f);
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
        ImGui::SetNextWindowSize(ImVec2(400,600), ImGuiSetCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(0, 100), ImGuiSetCond_FirstUseEver);
        // clang-format on

        updateSelected();

        if(ImGui::Begin("scene explorer", nullptr, window_flags)) {
            std::vector<oid> to_select;
            std::vector<oid> to_deselect;

            if(ImGui::TreeNodeEx((const void*)"obs", ImGuiTreeNodeFlags_DefaultOpen, "objects")) {
                static ImGuiTextFilter filter;
                filter.Draw("Filter (inc,-exc)");

                // recursive select/deselect
                std::function<void(oid, bool)> recursiveSelecter = [&](oid root, bool select) {
                    auto& root_obj = root.get();
                    if(select && !root_obj.has(selected_mixin_id))
                        to_select.push_back(root);
                    else if(root_obj.has(selected_mixin_id))
                        to_deselect.push_back(root);

                    // recurse through children
                    const auto& children = ::get_children(root_obj);
                    if(!children.empty())
                        for(const auto& c : children)
                            recursiveSelecter(c, select);
                };

                // recursive tree build
                std::function<void(oid, bool)> buildTree = [&](oid root, bool display) {
                    auto&              obj = root.get();
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
                        is_open = ImGui::TreeNodeEx((void*)(intptr_t)int16(root), node_flags,
                                                    name.c_str());
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

                for(const auto& curr : om.getObjects()) {
                    // recurse from those without a parent only
                    if(curr.second.implements(get_parent_msg)) {
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
                    JsonData state(10000);
                    state.startObject();
                    common::serialize_mixins(id.get(), "selected", state);
                    state.endObject();

                    HA_SUPPRESS_WARNINGS
                    comp_cmd.commands.push_back(
                            object_mutation_cmd({id, {"selected"}, state.data(), select}));
                    HA_SUPPRESS_WARNINGS_END
                };

                for(auto curr : to_select) {
                    add_mutate_command(curr, true);
                    curr.get().addMixin("selected");
                }
                for(auto curr : to_deselect) {
                    add_mutate_command(curr, false);
                    curr.get().remMixin("selected");
                }

                HA_SUPPRESS_WARNINGS
                add_command(comp_cmd);
                HA_SUPPRESS_WARNINGS_END

                //re-update the list for later usage
                updateSelected();
            }
        }
        ImGui::End();

        ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiSetCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(float(app.width() - 400), 0), ImGuiSetCond_FirstUseEver);

        if(ImGui::Begin("object attributes", nullptr, window_flags)) {
            for(auto& id : selected) {
                auto& obj = id.get();
                if(ImGui::TreeNodeEx((const void*)obj.name().c_str(),
                                     ImGuiTreeNodeFlags_DefaultOpen, obj.name().c_str())) {
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
        m_gizmo_state.cam.yfov          = glm::radians(45.0f);
        m_gizmo_state.screenspace_scale = 80.f; // 80px screenspace - or something like that
        glm::vec3 pos                   = tr::get_pos(World::get().camera());
        glm::quat rot                   = tr::get_rot(World::get().camera());
        m_gizmo_state.cam.position      = {pos.x, pos.y, pos.z};
        m_gizmo_state.cam.orientation   = {rot.x, rot.y, rot.z, rot.w};
        m_gizmo_ctx.update(m_gizmo_state);

        // update gizmo position to be between all selected objects
        if(!mouse_button_left_changed && !m_gizmo_state.mouse_left) {
            glm::vec3 avg_pos = {0, 0, 0};
            for(auto& curr : selected_with_gizmo)
                avg_pos += tr::get_pos(curr);
            avg_pos /= selected_with_gizmo.size();
            gizmo_transform.position = {avg_pos.x, avg_pos.y, avg_pos.z};
        }

        // record gizmo transform on start of usage
        if(mouse_button_left_changed) {
            if(m_gizmo_state.mouse_left) {
                gizmo_transform_last = gizmo_transform;
                for(auto& id : selected_with_gizmo)
                    sel::get_transform_on_gizmo_start(id) = tr::get_transform(id);
            }
        }

        // update gizmo
        tinygizmo::transform_gizmo("gizmo", m_gizmo_ctx, gizmo_transform);

        // if (probably) using gizmo - need this to really determine it: https://github.com/ddiakopoulos/tinygizmo/issues/6
        if(m_gizmo_state.mouse_left) {
            auto diff_pos = gizmo_transform.position - gizmo_transform_last.position;
            //auto diff_scl = gizmo_transform.scale - gizmo_transform_last.scale;
            //auto diff_rot = gizmo_transform.orientation - gizmo_transform_last.orientation;
            if(minalg::length(diff_pos) > 0) {
                for(auto& id : selected_with_gizmo) {
                    auto t = sel::get_transform_on_gizmo_start(id);
                    t.pos += glm::vec3(diff_pos.x, diff_pos.y, diff_pos.z);
                    tr::set_pos(id, t.pos);
                }
            }
        }

        // check if anything changed after release
        if(mouse_button_left_changed) {
            if(!m_gizmo_state.mouse_left) {
                handle_gizmo_changes_from_multi_selection();
            }
        }
        mouse_button_left_changed = false;

        m_gizmo_ctx.draw();
    }

    void handle_gizmo_changes_from_multi_selection() {
        compound_cmd comp_cmd;

        for(auto& id : selected_with_gizmo) {
            auto old_t = sel::get_transform_on_gizmo_start(id);
            auto new_t = tr::get_transform(id);
            if(old_t != new_t) {
                JsonData ov = command("tform", "t", old_t);
                JsonData nv = command("tform", "t", new_t);
                HA_SUPPRESS_WARNINGS
                comp_cmd.commands.push_back(attribute_changed_cmd({id, ov.data(), nv.data()}));
                HA_SUPPRESS_WARNINGS_END
                // update this - even though we havent started using the gizmo - or else this might break when deleting the object
                sel::get_transform_on_gizmo_start(id) = tr::get_transform(id);
            }
        }
        HA_SUPPRESS_WARNINGS
        if(!comp_cmd.commands.empty())
            add_command(comp_cmd);
        HA_SUPPRESS_WARNINGS_END
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

            // undo
            if(key == GLFW_KEY_Z && (mods & GLFW_MOD_CONTROL) && (action != GLFW_RELEASE)) {
                if(curr_undo_redo >= 0) {
                    printf("[UNDO] current action in undo/redo stack: %d (a total of %d actions)\n",
                           curr_undo_redo - 1, int(undo_redo_commands.size()));
                    handle_command(undo_redo_commands[curr_undo_redo--], true);
                }
            }

            // redo
            if(key == GLFW_KEY_Y && (mods & GLFW_MOD_CONTROL) && (action != GLFW_RELEASE)) {
                if(curr_undo_redo + 1 < int(undo_redo_commands.size())) {
                    printf("[REDO] current action in undo/redo stack: %d (a total of %d actions)\n",
                           curr_undo_redo + 1, int(undo_redo_commands.size()));
                    handle_command(undo_redo_commands[++curr_undo_redo], false);
                }
            }

            // delete selected objects
            if(key == GLFW_KEY_DELETE && (action != GLFW_RELEASE)) {
                if(!selected.empty()) {
                    handle_gizmo_changes_from_multi_selection();

                    compound_cmd comp_cmd;
                    comp_cmd.commands.reserve(selected.size() * 2);
                    for(auto& curr : selected) {
                        // get the list of mixin names
                        std::vector<const char*> mixins;
                        curr.get().get_mixin_names(mixins);
                        std::vector<std::string> mixin_names(mixins.size());
                        std::transform(mixins.begin(), mixins.end(), mixin_names.begin(),
                                       [](auto in) { return in; });

                        // serialize the state of the mixins
                        JsonData mixins_state(10000);
                        mixins_state.startObject();
                        common::serialize_mixins(curr.get(), nullptr, mixins_state);
                        mixins_state.endObject();

                        // serialize the state of the object itself
                        JsonData object_state(10000);
                        object_state.startObject();
                        object_state.append("\"\":");
                        serialize(curr.get(), object_state);
                        object_state.endObject();

                        HA_SUPPRESS_WARNINGS
                        comp_cmd.commands.push_back(object_mutation_cmd(
                                {curr, mixin_names, mixins_state.data(), false}));
                        comp_cmd.commands.push_back(
                                object_creation_cmd({curr, object_state.data(), false}));
                        HA_SUPPRESS_WARNINGS_END

                        ObjectManager::get().destroy(curr);
                    }
                    HA_SUPPRESS_WARNINGS
                    add_command(comp_cmd);
                    HA_SUPPRESS_WARNINGS_END

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
        if(command_var.type() == boost::typeindex::type_id<attribute_changed_cmd>()) {
            auto&       cmd = boost::get<attribute_changed_cmd>(command_var);
            auto&       val = undo ? cmd.old_val : cmd.new_val;
            const auto& doc = JsonData::parse(val);
            hassert(doc.is_valid());
            const auto root = doc.get_root();
            hassert(root.get_length() == 1);
            if(strcmp(root.get_object_key(0).data(), "") == 0)
                deserialize(cmd.e.get(), root.get_object_value(0)); // object attribute
            else
                common::deserialize_mixins(cmd.e, root); // mixin attribute
        } else if(command_var.type() == boost::typeindex::type_id<object_mutation_cmd>()) {
            auto& cmd = boost::get<object_mutation_cmd>(command_var);
            if((!cmd.added && undo) || (cmd.added && !undo)) {
                // add the mixins
                for(auto& mixin : cmd.mixins)
                    cmd.id.get().addMixin(mixin.c_str());
                // deserialize the mixins
                const auto& doc = JsonData::parse(cmd.mixins_state);
                hassert(doc.is_valid());
                common::deserialize_mixins(cmd.id, doc.get_root());
            } else {
                // remove the mixins
                for(auto& mixin : cmd.mixins)
                    cmd.id.get().remMixin(mixin.c_str());
            }
        } else if(command_var.type() == boost::typeindex::type_id<object_creation_cmd>()) {
            auto& cmd = boost::get<object_creation_cmd>(command_var);
            if((cmd.created && undo) || (!cmd.created && !undo)) {
                ObjectManager::get().destroy(cmd.id);
            } else {
                ObjectManager::get().createFromId(cmd.id);
                const auto& doc = JsonData::parse(cmd.object_state);
                hassert(doc.is_valid());
                deserialize(cmd.id.get(), doc.get_root().get_object_value(0)); // object attributes
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
        HA_SUPPRESS_WARNINGS
        add_command(attribute_changed_cmd({e, old_val, new_val}));
        HA_SUPPRESS_WARNINGS_END
    }
};

HA_SINGLETON_INSTANCE(editor);

HA_MIXIN_DEFINE(editor, Interface_editor);

#include <gen/editor.cpp.inl>
