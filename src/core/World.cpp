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
    set_scl(obj, {5, 5, 5});
}

void World::update() {
    Application& app = Application::get();
    float        dt  = app.dt();

    bgfx_dbg_text_clear(0, false);
    bgfx_dbg_text_printf(0, 1, 0x0f, "Frame: % 7.3f[ms]", double(dt) * 1000);

    // Set view and projection matrix for view 0.

    glm::mat4 view = get_view_matrix(m_camera);
    glm::mat4 proj = get_projection_matrix(m_camera);

    bgfx_set_view_transform(0, (float*)&view, (float*)&proj);
    // Set view 0 default viewport.
    bgfx_set_view_rect(0, 0, 0, uint16(app.width()), uint16(app.height()));
    bgfx_touch(0);

    auto& mixins = getMixins();
    mixins["camera"].update(dt);

    std::vector<renderPart> renderData;

    for(const auto& e : EntityManager::get().getEntities())
        if(e.second.implements(get_rendering_parts_msg))
            get_rendering_parts(e.second, renderData);
    for(const auto& data : renderData)
        meshSubmit(data.mesh.get(), 0, data.shader.get(), (float*)&data.transform
                   //, BGFX_STATE_DEFAULT
                   );

    mixins["editor"].update(dt);
}
