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
    bgfx::ProgramHandle                m_program;
    bgfx::VertexBufferHandle           m_vert_buf = BGFX_INVALID_HANDLE;
    bgfx::IndexBufferHandle            m_ind_buf  = BGFX_INVALID_HANDLE;

public:
    editor() {
        m_program = loadProgram("gizmo_vs", "gizmo_fs");
        bgfx::VertexDecl vert_decl;
        vert_decl.begin()
                .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
                .add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
                .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float)
                .end();

        m_gizmo_ctx.render = [&](const tinygizmo::geometry_mesh& r) {
            auto identity = glm::mat4(1.f);
            bgfx::setTransform((float*)&identity);

            m_verts.resize(r.vertices.size() * sizeof(tinygizmo::geometry_vertex));
            memcpy(m_verts.data(), r.vertices.data(), m_verts.size());
            m_inds.resize(r.triangles.size() * sizeof(r.triangles[0]));
            memcpy(m_inds.data(), r.triangles.data(), m_inds.size());

            // TODO: fix this! use dynamic or transient buffers (whatever that means)
            // and just update them instead of constantly recreating them
            if(isValid(m_vert_buf)) {
                bgfx::destroyVertexBuffer(m_vert_buf);
                bgfx::destroyIndexBuffer(m_ind_buf);
            }
            m_vert_buf = bgfx::createVertexBuffer(
                    bgfx::makeRef(m_verts.data(), uint32(m_verts.size())), vert_decl);
            m_ind_buf = bgfx::createIndexBuffer(bgfx::makeRef(m_inds.data(), uint32(m_inds.size())),
                                                BGFX_BUFFER_INDEX32);

            bgfx::setVertexBuffer(0, m_vert_buf);
            bgfx::setIndexBuffer(m_ind_buf);
            bgfx::setState(BGFX_STATE_DEFAULT);
            bgfx::submit(0, m_program);
        };
    }

    ~editor() {
        bgfx::destroyProgram(m_program);
        if(isValid(m_vert_buf)) {
            bgfx::destroyVertexBuffer(m_vert_buf);
            bgfx::destroyIndexBuffer(m_ind_buf);
        }
    }

    void update(float) {
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
