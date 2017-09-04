#include "World.h"

#include "Application.h"

#include "core/messages/messages.h"
#include "core/messages/messages_camera.h"
#include "core/messages/messages_rendering.h"

#include "core/registry/registry.h"

HA_SINGLETON_INSTANCE(World);

World::World()
        : Singleton<World>(this) {
    auto& om = ObjectManager::get();

    m_camera = om.create("camera");
    m_camera.get().addMixin("camera");

    // EXAMPLE: serialize and deserialize an object - may not work if the constructor of a mixin expects the presense of other mixins
    //JsonData state;
    //state.startObject();
    //common::serialize_mixins(m_camera, nullptr, state);
    //state.endObject();
    //om.destroy(m_camera);
    //om.createFromId(m_camera);
    //m_camera.get().setName("camera");
    //const auto& doc = JsonData::parse(state.data());
    //hassert(doc.is_valid());
    //auto root = doc.get_root();
    //for(size_t i = 0; i < root.get_length(); ++i)
    //    m_camera.get().addMixin(root.get_object_key(i).data());
    //common::deserialize_mixins(m_camera, root);

    m_editor.setName("editor");
    m_editor.addMixin("editor");

    auto& obj = om.create("with_mesh!").get();
    obj.addMixin("mesh");
    tr::set_scl(obj, {4, 4, 4});

    auto& obj2 = om.create("with_mesh_2!").get();
    obj2.addMixin("mesh");
    tr::set_scl(obj2, {2, 2, 2});
    tr::set_pos(obj2, {3, 0, 3});

    auto& obj3 = om.create("with_mesh_3!").get();
    obj3.addMixin("mesh");
    tr::set_scl(obj3, {3, 3, 3});
    tr::set_pos(obj3, {15, 0, -10});

    auto& copied = om.create().get();
    copied.copy_from(obj3);
    copied.setName("with_mesh_4   !!!");
    tr::move(copied, {0, 0, 5});

    auto& dummy1 = om.create("with_no_brain").get();
    auto& dummy2 = om.create("with_no_brain 2").get();
    auto& dummy3 = om.create("blabla").get();
    auto& dummy4 = om.create("dummy 4").get();
    auto& dummy5 = om.create("dummy with 5").get();

    set_parent(obj2, obj);

    set_parent(dummy1, obj);
    set_parent(dummy2, obj);

    set_parent(dummy3, dummy1);
    set_parent(dummy4, dummy1);

    set_parent(dummy5, obj2);
}

void World::update() {
    Application& app    = Application::get();
    auto&        mixins = getMixins();
    float        dt     = app.dt();

    bgfx_dbg_text_clear(0, false);
    bgfx_dbg_text_printf(0, 1, 0x0f, "Frame: % 7.3f[ms]", double(dt) * 1000);

    mixins["camera"].update(dt);
    yama::matrix view = cam::get_view_matrix(m_camera);
    yama::matrix proj = cam::get_projection_matrix(m_camera);

    // Set view and projection matrix for view 0.
    bgfx_set_view_transform(0, (float*)&view, (float*)&proj);
    bgfx_set_view_transform(1, (float*)&view, (float*)&proj);
    // Set view 0 default viewport.
    bgfx_set_view_rect(0, 0, 0, uint16(app.width()), uint16(app.height()));
    bgfx_set_view_rect(1, 0, 0, uint16(app.width()), uint16(app.height()));
    bgfx_touch(0);
    bgfx_touch(1);

    std::vector<renderPart> renderData;

    for(const auto& e : ObjectManager::get().getObjects())
        if(e.second.implements(rend::get_rendering_parts_msg))
            rend::get_rendering_parts(e.second, renderData);
    for(const auto& data : renderData) {
        if(data.mesh.isValid()) {
            meshSubmit(data.mesh.get(), 0, data.shader.get(), (const float*)&data.transform);
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
