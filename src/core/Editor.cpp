#include "Editor.h"

#include "core/Application.h"
#include "core/ObjectManager.h"
#include "core/GraphicsHelpers.h"
#include "messages/messages.h"

HA_SUPPRESS_WARNINGS
#include <GLFW/glfw3.h>
HA_SUPPRESS_WARNINGS_END

HA_SCOPED_SINGLETON_IMPLEMENT(Editor);

void Editor::init() {
    m_program = loadProgram("gizmo_vs", "gizmo_fs");
    m_vert_decl.begin()
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
        m_vert_buf = bgfx::createVertexBuffer(bgfx::makeRef(m_verts.data(), uint32(m_verts.size())),
                                              m_vert_decl);
        m_ind_buf = bgfx::createIndexBuffer(bgfx::makeRef(m_inds.data(), uint32(m_inds.size())),
                                            BGFX_BUFFER_INDEX32);

        bgfx::setVertexBuffer(0, m_vert_buf);
        bgfx::setIndexBuffer(m_ind_buf);
        bgfx::setState(BGFX_STATE_DEFAULT | BGFX_STATE_PT_TRISTRIP);
        bgfx::submit(0, m_program);
    };
}

void Editor::update() {
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

    static std::vector<eid> selected;

    if(ImGui::Begin("scene explorer", nullptr, window_flags)) {
        if(ImGui::TreeNode("objects")) {
            //auto hierarchical_id = dynamix::internal::domain::instance().get_mixin_id_by_name("hierarchical");

            for(const auto& curr : om.m_objects) {
                // recursive select/deselect
                std::function<void(eid, bool)> recursiveSelecter = [&](eid root, bool select) {
                    auto& obj = om.getObject(root);
                    auto  it  = std::find(selected.begin(), selected.end(), root);
                    if(select) {
                        if(it == selected.end())
                            selected.push_back(root);
                    } else {
                        if(it != selected.end())
                            selected.erase(it);
                    }
                    obj.select(select);

                    // recurse through children
                    const auto& children = ::get_children(obj);
                    if(children.size() > 0)
                        for(const auto& c : children)
                            recursiveSelecter(c, select);
                };

                // recursive tree build
                std::function<void(eid)> buildTree = [&](eid root) {
                    auto&              obj = om.getObject(root);
                    ImGuiTreeNodeFlags node_flags =
                            ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
                            (obj.selected() ? ImGuiTreeNodeFlags_Selected : 0);

                    const auto& children = ::get_children(obj);
                    if(children.size() == 0)
                        node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

                    auto name      = obj.name() + " (" + std::to_string(int(obj.id())) + ")";
                    bool node_open = ImGui::TreeNodeEx((void*)(intptr_t) int(root), node_flags,
                                                       name.c_str());

                    if(ImGui::IsItemClicked()) {
                        bool shouldSelect = !obj.selected();

                        if(ImGui::GetIO().KeyShift) {
                            recursiveSelecter(root, shouldSelect);
                        } else if(ImGui::GetIO().KeyCtrl) {
                            obj.select(shouldSelect);
                            if(shouldSelect)
                                selected.push_back(root);
                            else
                                selected.erase(std::find(selected.begin(), selected.end(), root));
                        } else if(!obj.selected()) {
                            for(auto& it : selected) {
                                om.getObject(it).select(false);
                            }
                            selected.clear();
                            obj.select(shouldSelect);
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
            auto& obj = om.getObject(id);
            if(ImGui::TreeNode(obj.name().c_str())) {
                if(obj.implements(imgui_bind_properties_msg)) {
                    imgui_bind_properties(obj);
                }

                ImGui::TreePop();
            }
        }
    }
    ImGui::End();

    //ImGui::ShowTestWindow();

    m_gizmo_state.viewport_size = {float(app.width()), float(app.height())};
    m_gizmo_state.cam.near_clip = 0.1f;
    m_gizmo_state.cam.far_clip  = 1000.f;
    m_gizmo_state.cam.yfov      = glm::radians(45.0f);
    glm::vec3 pos = get_pos(ObjectManager::get().getObject(ObjectManager::get().m_camera));
    glm::quat rot = get_rot(ObjectManager::get().getObject(ObjectManager::get().m_camera));
    m_gizmo_state.cam.position    = {pos.x, pos.y, pos.z};
    m_gizmo_state.cam.orientation = {rot.x, rot.y, rot.z, rot.w};

    m_gizmo_ctx.update(m_gizmo_state);
    tinygizmo::transform_gizmo("xform-example-gizmo", m_gizmo_ctx, m_transform);
    m_gizmo_ctx.draw();
}

void Editor::process_event(const InputEvent& ev) {
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
