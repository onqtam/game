#include "World.h"

#include "Application.h"

#include "core/messages/messages.h"
#include "core/messages/messages_camera.h"
#include "core/messages/messages_rendering.h"

#include "core/registry/registry.h"

HA_SINGLETON_INSTANCE(World);

World::World()
        : Singleton<World>(this) {
    auto& em = EntityManager::get();

    m_camera = em.newEntityId("camera");
    m_camera.get().addMixin("camera");

    m_editor.setName("editor");
    m_editor.addMixin("editor");

    auto& obj = em.newEntity("with_mesh!");
    obj.addMixin("mesh");
    tr::set_scl(obj, {5, 5, 5});

    auto& obj2 = em.newEntity("with_mesh_2!");
    obj2.addMixin("mesh");
    tr::set_scl(obj2, {7, 7, 7});
    tr::set_pos(obj2, {20, 0, 20});
}

void World::update() {
    Application& app    = Application::get();
    auto&        mixins = getMixins();
    float        dt     = app.dt();

    bgfx_dbg_text_clear(0, false);
    bgfx_dbg_text_printf(0, 1, 0x0f, "Frame: % 7.3f[ms]", double(dt) * 1000);

    mixins["camera"].update(dt);
    glm::mat4 view = cam::get_view_matrix(m_camera);
    glm::mat4 proj = cam::get_projection_matrix(m_camera);

    // Set view and projection matrix for view 0.
    bgfx_set_view_transform(0, (float*)&view, (float*)&proj);
    // Set view 0 default viewport.
    bgfx_set_view_rect(0, 0, 0, uint16(app.width()), uint16(app.height()));
    bgfx_touch(0);

    std::vector<renderPart> renderData;

    for(const auto& e : EntityManager::get().getEntities())
        if(e.second.implements(rend::get_rendering_parts_msg))
            rend::get_rendering_parts(e.second, renderData);
    for(const auto& data : renderData) {
        if(data.mesh.isValid()) {
            meshSubmit(data.mesh.get(), 0, data.shader.get(), (const float*)&data.transform
                       //, BGFX_STATE_DEFAULT
                       );
        } else if(data.geom.isValid()) {
            bgfx_set_transform((const float*)&data.transform, 1);
            bgfx_set_state(BGFX_STATE_DEFAULT | data.geom.get().state, 0);

            bgfx_set_vertex_buffer(0, data.geom.get().vbh, 0, UINT32_MAX);
            if(data.geom.get().ibh.idx != BGFX_INVALID_HANDLE)
                bgfx_set_index_buffer(data.geom.get().ibh, 0, UINT32_MAX);
            bgfx_submit(0, data.shader.get(), 0, false);
        }
    }

    mixins["editor"].update(dt);
}
