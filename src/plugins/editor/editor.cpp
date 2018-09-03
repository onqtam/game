#include "editor.h"

#include "core/serialization/serialization_common.h"
#include "core/serialization/serialization_2.h"
#include "core/imgui/imgui_bindings_common.h"

#include "core/messages/messages_rendering.h"
#include "core/World.h"

editor::editor()
        : Singleton(this) {
    m_grid = GeomMan::get().get("", createGrid, 20, 20, World::get().width(), World::get().height(),
                                colors::green);
    m_grid_shader = ShaderMan::get().get("cubes");

    m_program = ShaderMan::get().get("gizmo");

    m_gizmo_ctx.render = [&](const tinygizmo::geometry_mesh& r) {
        //if (!m_gizmo_verts.empty()) return;
        m_gizmo_verts.resize(r.vertices.size());
        memcpy(m_gizmo_verts.data(), r.vertices.data(),
               m_gizmo_verts.size() * sizeof(tinygizmo::geometry_vertex));
        m_gizmo_inds.resize(r.triangles.size() * 3);
        memcpy(m_gizmo_inds.data(), r.triangles.data(), m_gizmo_inds.size() * sizeof(uint32));
        m_render_gizmo = true;
    };
}

editor::~editor() {}

void editor::update(float) {
    update_gui();
    update_gizmo();
}

void editor::get_rendering_parts(std::vector<renderPart>& out) const {
    out.push_back({m_grid, TempMesh(), yama::matrix::identity()});
    if(m_render_gizmo) {
        renderPart part;
        part.tmpMesh.vertices = &m_gizmo_verts;
        part.tmpMesh.indices  = &m_gizmo_inds;
        part.transform        = yama::matrix::identity();
        out.push_back(part);
        m_render_gizmo = false;
    }
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
    state.fwrite("level.json");
}

HA_MIXIN_DEFINE(editor, Interface_editor& rend::get_rendering_parts_msg)

void selected::submit_aabb_rec(const Object& curr, std::vector<renderPart>& out) {
    // if object has a bbox - submit it
    if(curr.implements(rend::get_aabb_msg)) {
        auto diag  = rend::get_aabb(curr).getDiagonal();
        auto color = curr.has<selected>() ? colors::green : colors::light_green;
        auto geom  = GeomMan::get().get("", createBox, diag.x, diag.y, diag.z, color);
        // auto shader = ShaderMan::get().get("cubes");
        out.push_back({geom, TempMesh(), curr.get_transform().as_mat()});
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
