#include "editor_gen.h"

#include "core/messages/messages.h"
#include "core/messages/messages_editor.h"
#include "core/Application.h"
#include "core/World.h"
#include "core/GraphicsHelpers.h"
#include "core/InputEvent.h"

#include <iostream>

HA_SUPPRESS_WARNINGS
#include <boost/range/adaptor/reversed.hpp>

#include <GLFW/glfw3.h>
HA_SUPPRESS_WARNINGS_END

template <typename T>
JsonData command(const char* mixin_name, const char* prop, const T& data) {
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

//class editor;
//void serialize(const editor& src, JsonData& out);
//void deserialize(editor& dest, const sajson::value& val);

typedef decltype(editor_gen::undo_redo_commands)::value_type command_variant_type;

class editor : public editor_gen,
               public UpdatableMixin<editor>,
               public InputEventListener,
               public Singleton<editor>
{
    HA_SINGLETON(editor);
    HA_MESSAGES_IN_MIXIN(editor)

    // these members are OK to not be serialized because they are constantly updated - for all other members use the .mix file!
    tinygizmo::gizmo_application_state m_gizmo_state;
    tinygizmo::gizmo_context           m_gizmo_ctx;
    std::vector<char>                  m_verts;
    std::vector<char>                  m_inds;
    ShaderHandle                       m_program;
    bgfx_vertex_decl                   vd;
    bgfx_vertex_buffer_handle          m_vert_buf = {BGFX_INVALID_HANDLE};
    bgfx_index_buffer_handle           m_ind_buf  = {BGFX_INVALID_HANDLE};

    GeomHandle   m_grid;
    ShaderHandle m_grid_shader;

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
        auto& em  = ObjectManager::get();

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

        if(ImGui::Begin("scene explorer", nullptr, window_flags)) {
            if(ImGui::TreeNode("objects")) {
                auto selected_mixin_id =
                        dynamix::internal::domain::instance().get_mixin_id_by_name("selected");

                for(const auto& curr : em.getEntities()) {
                    // recursive select/deselect
                    std::function<void(oid, bool)> recursiveSelecter = [&](oid root, bool select) {
                        auto& obj = root.get();
                        auto  it  = std::find(selected.begin(), selected.end(), root);
                        if(select) {
                            if(it == selected.end()) {
                                selected.push_back(root);
                                obj.addMixin("selected");
                            }
                        } else {
                            if(it != selected.end()) {
                                selected.erase(it);
                                obj.remMixin("selected");
                            }
                        }

                        // recurse through children
                        const auto& children = ::get_children(obj);
                        if(children.size() > 0)
                            for(const auto& c : children)
                                recursiveSelecter(c, select);
                    };

                    // recursive tree build
                    std::function<void(oid)> buildTree = [&](oid root) {
                        auto&              obj = root.get();
                        ImGuiTreeNodeFlags node_flags =
                                ImGuiTreeNodeFlags_OpenOnArrow |
                                ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                (obj.has(selected_mixin_id) ? ImGuiTreeNodeFlags_Selected : 0);

                        const auto& children = ::get_children(obj);
                        if(children.size() == 0)
                            node_flags |=
                                    ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

                        auto name      = obj.name() + " (" + std::to_string(int16(obj.id())) + ")";
                        bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)int16(root), node_flags,
                                                           name.c_str());

                        if(ImGui::IsItemClicked()) {
                            bool shouldSelect = !obj.has(selected_mixin_id);

                            if(ImGui::GetIO().KeyShift) {
                                recursiveSelecter(root, shouldSelect);
                            } else if(ImGui::GetIO().KeyCtrl) {
                                if(shouldSelect) {
                                    selected.push_back(root);
                                    obj.addMixin("selected");
                                } else {
                                    selected.erase(
                                            std::find(selected.begin(), selected.end(), root));
                                    obj.remMixin("selected");
                                }
                            } else if(!obj.has(selected_mixin_id)) {
                                for(auto& it : selected) {
                                    it.get().remMixin("selected");
                                }
                                selected.clear();
                                if(shouldSelect)
                                    obj.addMixin("selected");
                                else
                                    obj.remMixin("selected");
                                selected.push_back(root);
                            }
                        }

                        if(node_open && children.size() > 0) {
                            for(const auto& c : children)
                                buildTree(c);
                            ImGui::TreePop();
                        }
                    };

                    // recurse from those without a parent only
                    if(curr.second.implements(get_parent_msg)) {
                        if(::get_parent(curr.second) == oid::invalid())
                            buildTree(curr.second.id());
                    }
                }
                ImGui::TreePop();
            }
        }
        ImGui::End();

        ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiSetCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(float(app.width() - 400), 0), ImGuiSetCond_FirstUseEver);

        if(ImGui::Begin("object attributes", nullptr, window_flags)) {
            for(auto& id : selected) {
                auto& obj = id.get();
                if(ImGui::TreeNode(obj.name().c_str())) {
                    if(obj.implements(common::imgui_bind_attributes_msg)) {
                        common::imgui_bind_attributes(obj);
                    }

                    ImGui::TreePop();
                }
            }
        }
        ImGui::End();

        ImGui::ShowTestWindow();

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

        for(auto& id : selected) {
            auto& obj = id.get();
            auto& t   = sel::get_gizmo_transform(obj);

            if(obj.implements(sel::no_gizmo_msg))
                continue;

            // record transform after press
            if(mouse_button_left_changed) {
                if(m_gizmo_state.mouse_left) {
                    sel::get_last_stable_gizmo_transform(obj) = t;
                }
            }

            // update transform
            tinygizmo::transform_gizmo(obj.name(), m_gizmo_ctx, t);

            // check if anything changed after release
            if(mouse_button_left_changed) {
                if(!m_gizmo_state.mouse_left) {
                    auto last = sel::get_last_stable_gizmo_transform(obj);
                    // NOTE that these also get triggered when I move the object with the float drags of the transform...
                    if(last.position != t.position) {
                        JsonData ov = command("transform", "pos", *(glm::vec3*)&last.position);
                        JsonData nv = command("transform", "pos", *(glm::vec3*)&t.position);
                        edit::add_changed_attribute(ha_this, obj.id(), ov.data(), nv.data());
                    }
                    if(last.orientation != t.orientation) {
                        JsonData ov = command("transform", "rot", *(glm::quat*)&last.orientation);
                        JsonData nv = command("transform", "rot", *(glm::quat*)&t.orientation);
                        edit::add_changed_attribute(ha_this, obj.id(), ov.data(), nv.data());
                    }
                    if(last.scale != t.scale) {
                        JsonData ov = command("transform", "scl", *(glm::vec3*)&last.scale);
                        JsonData nv = command("transform", "scl", *(glm::vec3*)&t.scale);
                        edit::add_changed_attribute(ha_this, obj.id(), ov.data(), nv.data());
                    }
                }

                mouse_button_left_changed = false;
            }
            tr::set_pos(obj, (glm::vec3&)t.position);
            tr::set_scl(obj, (glm::vec3&)t.scale);
            tr::set_rot(obj, (glm::quat&)t.orientation);
        }

        m_gizmo_ctx.draw();
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
                if(selected.size() > 0) {
                    compound_cmd_gen comp_cmd;
                    comp_cmd.commands.reserve(selected.size() * 2);
                    for(auto& curr : selected) {
                        // remove selected component ---> TEMP HACK!
                        curr.get().remMixin("selected");

                        // get the list of mixin names
                        std::vector<const char*> mixins;
                        curr.get().get_mixin_names(mixins);
                        std::vector<std::string> mixin_names(mixins.size());
                        std::transform(mixins.begin(), mixins.end(), mixin_names.begin(),
                                       [](auto in) { return in; });

                        // serialize the state of the mixins
                        JsonData state;
                        state.reserve(10000);
                        state.startObject();
                        common::serialize(curr.get(), state);
                        state.endObject();

                        HA_SUPPRESS_WARNINGS
                        comp_cmd.commands.push_back(
                                entity_mutation_cmd_gen({curr, mixin_names, state.data(), false}));
                        comp_cmd.commands.push_back(
                                entity_creation_cmd_gen({curr, curr.get().name(), false}));
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

    void handle_command(command_variant_type& command_variant, bool undo) {
        if(command_variant.which() == 0) { // attribute changed
            auto&       cmd = boost::get<attribute_changed_cmd_gen>(command_variant);
            auto&       val = undo ? cmd.old_val : cmd.new_val;
            JsonData    state(val);
            const auto& doc = state.parse();
            hassert(doc.is_valid());
            const auto root = doc.get_root();
            common::deserialize(cmd.e, root);
        } else if(command_variant.which() == 1) { // entity mutation
            auto& cmd = boost::get<entity_mutation_cmd_gen>(command_variant);
            if((!cmd.added && undo) || (cmd.added && !undo)) {
                // add the mixins
                for(auto& mixin : cmd.mixins)
                    cmd.id.get().addMixin(mixin.c_str());
                // deserialize the mixins
                JsonData    state(cmd.mixins_state);
                const auto& doc = state.parse();
                hassert(doc.is_valid());
                const auto root = doc.get_root();
                common::deserialize(cmd.id, root);
            } else {
                // remove the mixins
                for(auto& mixin : cmd.mixins)
                    cmd.id.get().remMixin(mixin.c_str());
            }
        } else if(command_variant.which() == 2) { // entity creation
            auto& cmd = boost::get<entity_creation_cmd_gen>(command_variant);
            if((cmd.created && undo) || (!cmd.created && !undo)) {
                ObjectManager::get().destroy(cmd.id);
            } else {
                ObjectManager::get().createFromId(cmd.id, cmd.name);
            }
        } else if(command_variant.which() == 3) { // compound
            auto& cmd = boost::get<compound_cmd_gen>(command_variant);
            if(undo)
                for(auto& curr : boost::adaptors::reverse(cmd.commands))
                    handle_command(curr, undo);
            else
                for(auto& curr : cmd.commands)
                    handle_command(curr, undo);
        }
    }

    void add_command(const command_variant_type& command) {
        if(undo_redo_commands.size())
            undo_redo_commands.erase(undo_redo_commands.begin() + 1 + curr_undo_redo,
                                     undo_redo_commands.end());

        undo_redo_commands.push_back(command);

        ++curr_undo_redo;
        printf("num actions in undo/redo stack: %d\n", curr_undo_redo);
    }

    void add_changed_attribute(oid e, const json_buf& old_val, const json_buf& new_val) {
        HA_SUPPRESS_WARNINGS
        add_command(attribute_changed_cmd_gen({e, old_val, new_val}));
        HA_SUPPRESS_WARNINGS_END
    }
};

//void serialize(const editor& src, JsonData& out) {
//    out.startObject();
//    serialize(src, out, false);
//    HA_SERIALIZE_VARIABLE("selected", src.selected);
//    out.endObject();
//}
//
//void deserialize(editor& dest, const sajson::value& val) {
//    const size_t val_len = val.get_length();
//    for(size_t i = 0; i < val_len; ++i) {
//        HA_DESERIALIZE_VARIABLE("selected", dest.selected);
//        HA_DESERIALIZE_VARIABLE("undo_redo_commands", dest.undo_redo_commands);
//        HA_DESERIALIZE_VARIABLE("curr_undo_redo", dest.curr_undo_redo);
//        HA_DESERIALIZE_VARIABLE("mouse_button_left_changed", dest.mouse_button_left_changed);
//    }
//}

HA_SINGLETON_INSTANCE(editor);

// defining it as without codegen for convenience - the singleton macro adds a dummy member variable
// and also tinygizmo::gizmo_context cannot be properly serialized because it uses the pimpl idiom
HA_MIXIN_DEFINE_WITHOUT_CODEGEN(editor,
                                common::serialize_msg& common::deserialize_msg& Interface_editor);
