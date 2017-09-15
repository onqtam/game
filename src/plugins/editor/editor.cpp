#include "editor.h"

#include "core/serialization/serialization_common.h"
#include "core/serialization/serialization_2.h"
#include "core/imgui/imgui_bindings_common.h"

#include "core/messages/messages_editor.h"
#include "core/World.h"

HA_SUPPRESS_WARNINGS
#include <GLFW/glfw3.h>
HA_SUPPRESS_WARNINGS_END

HA_GCC_SUPPRESS_WARNING("-Wzero-as-null-pointer-constant") // because of boost::variant's ctor

// until the allocator model of dynamix is extended we shall update this list manually like this
void editor::update_selected() {
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

editor::editor()
        : Singleton(this) {
    m_grid = GeomMan::get().get("", createGrid, 20, 20, World::get().width(), World::get().height(),
                                colors::green);
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
        m_ind_buf = bgfx_create_index_buffer(bgfx_make_ref(m_inds.data(), uint32(m_inds.size())),
                                             BGFX_BUFFER_INDEX32);

        bgfx_set_vertex_buffer(0, m_vert_buf, 0, UINT32_MAX);
        bgfx_set_index_buffer(m_ind_buf, 0, UINT32_MAX);
        bgfx_set_state(BGFX_STATE_DEFAULT, 0);
        bgfx_submit(1, m_program.get(), 0, false);
    };
}

editor::~editor() {
    if(m_vert_buf.idx != BGFX_INVALID_HANDLE) {
        bgfx_destroy_vertex_buffer(m_vert_buf);
        bgfx_destroy_index_buffer(m_ind_buf);
    }
}

void editor::update(float) {
    // draw grid
    auto identity = yama::matrix::identity();
    bgfx_set_transform((float*)&identity, 1);
    bgfx_set_vertex_buffer(0, m_grid.get().vbh, 0, UINT32_MAX);
    bgfx_set_state(BGFX_STATE_DEFAULT | m_grid.get().state, 0);
    bgfx_submit(0, m_grid_shader.get(), 0, false);

    update_gui();
    update_gizmo();
}

void editor::process_event(const InputEvent& ev) {
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
        if(key == GLFW_KEY_G && (mods & GLFW_MOD_CONTROL) && (action == GLFW_PRESS))
            group_selected();

        // ungroup
        if(key == GLFW_KEY_U && (mods & GLFW_MOD_CONTROL) && (action == GLFW_PRESS))
            ungroup_selected();

        // duplicate
        if(key == GLFW_KEY_D && (mods & GLFW_MOD_CONTROL) && (action == GLFW_PRESS))
            duplicate_selected();

        // delete selected objects
        if(key == GLFW_KEY_DELETE && (action != GLFW_RELEASE))
            delete_selected();
    } else if(ev.type == InputEvent::BUTTON) {
        if(ev.button.button == 0)
            m_gizmo_state.mouse_left = (ev.button.action != GLFW_RELEASE);
        mouse_button_left_changed = true;
    }
}

HA_GCC_SUPPRESS_WARNING_END

HA_SINGLETON_INSTANCE(editor);

HA_MIXIN_DEFINE(editor, Interface_editor);

#include <gen/editor.h.inl>
