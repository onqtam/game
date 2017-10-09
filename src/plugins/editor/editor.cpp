#include "editor.h"

#include "core/serialization/serialization_common.h"
#include "core/serialization/serialization_2.h"
#include "core/imgui/imgui_bindings_common.h"

#include "core/messages/messages_rendering.h"
#include "core/World.h"

// until the allocator model of dynamix is extended we shall update this list manually like this
void editor::update_selected() {
    m_selected.clear();
    selected_with_gizmo.clear();
    for(auto& curr : ObjectManager::get().getObjects()) {
        if(curr.second.has<selected>()) {
            m_selected.push_back(curr.second.id());
            if(curr.second.implements(no_gizmo_msg))
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
    if(ev.type == InputEvent::MOUSE) {
        m_gizmo_state.cursor = {float(ev.mouse.x), float(ev.mouse.y)};
    } else if(ev.type == InputEvent::KEY) {
        auto key    = ev.key.key;
        auto action = ev.key.action;
        auto mods   = ev.key.mods;
        if(key == HA_KEY_LEFT_CONTROL)
            m_gizmo_state.hotkey_ctrl = (action != KeyAction::Release);
        if(key == HA_KEY_L)
            m_gizmo_state.hotkey_local = (action != KeyAction::Release);
        if(key == HA_KEY_T)
            m_gizmo_state.hotkey_translate = (action != KeyAction::Release);
        if(key == HA_KEY_R)
            m_gizmo_state.hotkey_rotate = (action != KeyAction::Release);
        if(key == HA_KEY_S)
            m_gizmo_state.hotkey_scale = (action != KeyAction::Release);

        // undo - with repeat
        if(key == HA_KEY_Z && (mods & HA_MOD_CONTROL) && (action != KeyAction::Release))
            undo();

        // redo - with repeat
        if(key == HA_KEY_Y && (mods & HA_MOD_CONTROL) && (action != KeyAction::Release))
            redo();

        // group
        if(key == HA_KEY_G && (mods & HA_MOD_CONTROL) && (action == KeyAction::Press))
            group_selected();

        // ungroup
        if(key == HA_KEY_U && (mods & HA_MOD_CONTROL) && (action == KeyAction::Press))
            ungroup_selected();

        // duplicate
        if(key == HA_KEY_D && (mods & HA_MOD_CONTROL) && (action == KeyAction::Press))
            duplicate_selected();

        // save
        if(key == HA_KEY_S && (mods & HA_MOD_CONTROL) && (action == KeyAction::Press))
            save();

        // delete selected objects
        if(key == HA_KEY_DELETE && (action != KeyAction::Release))
            delete_selected();
    } else if(ev.type == InputEvent::BUTTON) {
        if(ev.button.button == MouseButton::Left)
            m_gizmo_state.mouse_left = (ev.button.action != ButtonAction::Release);
        mouse_button_left_changed = true;
    }
}

HA_SINGLETON_INSTANCE(editor);

void editor::save() {
    auto& objects = ObjectManager::get().getObjects();

    JsonData state;
    state.startObject();

    state.addKey("objects");
    state.startArray();

    for(auto& p : objects) {
        auto& curr = p.second;

        state.startObject();
        state.addKey("id");
        auto id_str = std::to_string(oid::internal_type(curr.id()));
        state.append(id_str.c_str(), id_str.size());
        state.addComma();
        state.addKey("state");
        serialize(curr, state);
        state.addComma();
        state.addKey("mixins");
        state.startObject();
        if(curr.implements(common::serialize_mixins_msg))
            common::serialize_mixins(curr, nullptr, state);
        state.endObject();
        state.endObject();

        state.addComma();
    }

    state.endArray();
    state.endObject();

    state.prettify();

    auto f = fopen("level.json", "wb");
    fwrite(state.data().data(), 1, state.size(), f);
    fclose(f);
}

HA_MIXIN_DEFINE(editor, Interface_editor)

void selected::submit_aabb_rec(const Object& curr, std::vector<renderPart>& out) {
    // if object has a bbox - submit it
    if(curr.implements(rend::get_aabb_msg)) {
        auto diag   = rend::get_aabb(curr).getDiagonal();
        auto color  = curr.has<selected>() ? colors::green : colors::light_green;
        auto geom   = GeomMan::get().get("", createBox, diag.x, diag.y, diag.z, color);
        auto shader = ShaderMan::get().get("cubes");
        out.push_back({{}, geom, shader, curr.get_transform().as_mat()});
    }
    // recurse through children
    auto& children = curr.get_children();
    for(auto& child_id : children) {
        auto& child = child_id.obj();
        // if child is not selected - to avoid rendering the same bbox multiple times
        if(!child.has<selected>())
            submit_aabb_rec(child, out);
    }
}

HA_MIXIN_DEFINE(selected, rend::get_rendering_parts_msg)

#include <gen/editor.h.inl>
