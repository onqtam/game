#include "editor_gen.h"

#include "core/messages/messages.h"
#include "core/Application.h"
#include "core/ObjectManager.h"
#include "core/GraphicsHelpers.h"
#include "core/InputEvent.h"

HA_SUPPRESS_WARNINGS
#include <GLFW/glfw3.h>
HA_SUPPRESS_WARNINGS_END

class editor : public editor_gen, public UpdatableMixin<editor>, public InputEventListener
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
    editor() {
        m_grid        = GeomMan::get().get("", createGrid, 10, 10, 100.f, 100.f, 0xffffffff);
        m_grid_shader = ShaderMan::get().get("cubes");

        m_program     = ShaderMan::get().get("gizmo");
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
        bgfx_set_transform((float*)&glm::mat4(1), 1);
        bgfx_set_vertex_buffer(0, m_grid.get().vbh, 0, UINT32_MAX);
        bgfx_set_state(BGFX_STATE_DEFAULT | m_grid.get().state, 0);
        bgfx_submit(0, m_grid_shader.get(), 0, false);

        auto& app = Application::get();
        auto& em  = EntityManager::get();

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

        static std::vector<eid> selected;

        if(ImGui::Begin("scene explorer", nullptr, window_flags)) {
            if(ImGui::TreeNode("objects")) {
                auto selected_mixin_id =
                        dynamix::internal::domain::instance().get_mixin_id_by_name("selected");

                for(const auto& curr : em.getEntities()) {
                    // recursive select/deselect
                    std::function<void(eid, bool)> recursiveSelecter = [&](eid root, bool select) {
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
                    std::function<void(eid)> buildTree = [&](eid root) {
                        auto&              obj = root.get();
                        ImGuiTreeNodeFlags node_flags =
                                ImGuiTreeNodeFlags_OpenOnArrow |
                                ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                (obj.has(selected_mixin_id) ? ImGuiTreeNodeFlags_Selected : 0);

                        const auto& children = ::get_children(obj);
                        if(children.size() == 0)
                            node_flags |=
                                    ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

                        auto name      = obj.name() + " (" + std::to_string(int(obj.id())) + ")";
                        bool node_open = ImGui::TreeNodeEx((void*)(intptr_t) int(root), node_flags,
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
                        if(::get_parent(curr.second) == eid::invalid())
                            buildTree(curr.second.id());
                    }
                }
                ImGui::TreePop();
            }
        }
        ImGui::End();

        ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiSetCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(float(app.width() - 400), 0), ImGuiSetCond_FirstUseEver);

        if(ImGui::Begin("object properties", nullptr, window_flags)) {
            for(auto& id : selected) {
                auto& obj = id.get();
                if(ImGui::TreeNode(obj.name().c_str())) {
                    if(obj.implements(imgui_bind_properties_msg)) {
                        ::imgui_bind_properties(obj);
                    }

                    ImGui::TreePop();
                }
            }
        }
        ImGui::End();

        //ImGui::ShowTestWindow();

        m_gizmo_state.viewport_size   = {float(app.width()), float(app.height())};
        m_gizmo_state.cam.near_clip   = 0.1f;
        m_gizmo_state.cam.far_clip    = 1000.f;
        m_gizmo_state.cam.yfov        = glm::radians(45.0f);
        glm::vec3 pos                 = get_pos(ObjectManager::get().m_camera);
        glm::quat rot                 = get_rot(ObjectManager::get().m_camera);
        m_gizmo_state.cam.position    = {pos.x, pos.y, pos.z};
        m_gizmo_state.cam.orientation = {rot.x, rot.y, rot.z, rot.w};

        m_gizmo_ctx.update(m_gizmo_state);

        for(auto& id : selected) {
            auto& obj = id.get();
            auto& t   = get_gizmo_transform(obj);

            // temp hack
            auto camera_mixin_id =
                    dynamix::internal::domain::instance().get_mixin_id_by_name("camera");
            if(obj.has(camera_mixin_id))
                continue;

            tinygizmo::transform_gizmo(obj.name(), m_gizmo_ctx, t);
            set_pos(obj, (glm::vec3&)t.position);
            set_scl(obj, (glm::vec3&)t.scale);
            set_rot(obj, (glm::quat&)t.orientation);
        }

        m_gizmo_ctx.draw();
    }

    void process_event(const InputEvent& ev) {
        if(ev.type == InputEvent::MOTION) {
            m_gizmo_state.cursor = {float(ev.motion.x), float(ev.motion.y)};
        } else if(ev.type == InputEvent::KEY) {
            auto key    = ev.key.key;
            auto action = ev.key.action;
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
        } else if(ev.type == InputEvent::BUTTON) {
            if(ev.button.button == 0)
                m_gizmo_state.mouse_left = (ev.button.action != GLFW_RELEASE);
        }
    }
};

// defining it as without codegen for convenience - the singleton macro adds a dummy member variable
// and also tinygizmo::gizmo_context cannot be properly serialized because it uses the pimpl idiom
HA_MIXIN_DEFINE_WITHOUT_CODEGEN(editor, serialize_msg& deserialize_msg);
