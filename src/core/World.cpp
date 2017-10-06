#include "World.h"

#include "Application.h"

#include "core/messages/messages_camera.h"
#include "core/messages/messages_rendering.h"

HA_SINGLETON_INSTANCE(World);

World::World()
        : Singleton<World>(this) {
    auto& om = ObjectManager::get();

    m_camera = om.create("camera").id();
    //m_camera.obj().addMixin("maya_camera");
    m_camera.obj().addMixin("gameplay_camera");

    // EXAMPLE: serialize and deserialize an object
    //JsonData state;
    //state.startObject();
    //common::serialize_mixins(m_camera.obj(), nullptr, state);
    //state.endObject();
    //om.destroy(m_camera);
    //om.createFromId(m_camera);
    //m_camera.obj().setName("camera");
    //const auto& doc = state.parse();
    //hassert(doc.is_valid());
    //auto root = doc.get_root();
    //for(size_t i = 0; i < root.get_length(); ++i)
    //    m_camera.obj().addMixin(root.get_object_key(i).data());
    //common::deserialize_mixins(m_camera.obj(), root);

    m_editor = om.create("editor").id();
    m_editor.obj().addMixin("editor");

    auto& center = om.create("0,0,0");
    center.addMixin("mesh");

    auto& obj = om.create("with_mesh!");
    obj.addMixin("mesh");
    obj.set_scl({4, 4, 4});

    auto& obj2 = om.create("with_mesh_2!");
    obj2.addMixin("mesh");
    obj2.set_scl({2, 2, 2});
    obj2.set_pos({3, 0, 3});

    auto& obj3 = om.create("with_mesh_3!");
    obj3.addMixin("mesh");
    obj3.set_scl({3, 3, 3});
    obj3.set_pos({15, 0, -10});

    auto& copied = om.create();
    copied.copy_from(obj3);
    copied.setName("with_mesh_4   !!!");
    copied.move_local({0, 0, 5});

    auto& dummy1 = om.create("with_no_brain");
    auto& dummy2 = om.create("with_no_brain 2");
    auto& dummy3 = om.create("blabla");
    auto& dummy4 = om.create("dummy 4");
    auto& dummy5 = om.create("dummy with 5");

    obj2.set_parent(obj.id());

    dummy1.set_parent(obj.id());
    dummy2.set_parent(obj.id());

    dummy3.set_parent(dummy1.id());
    dummy4.set_parent(dummy1.id());

    dummy5.set_parent(obj2.id());
}

void World::update() {
    Application& app    = Application::get();
    auto&        mixins = getAllMixins();
    float        dt     = app.dt();

    bgfx_dbg_text_clear(0, false);
    bgfx_dbg_text_printf(0, 1, 0x0f, "Frame: % 7.3f[ms]", double(dt) * 1000);

    mixins["gameplay_camera"].update(dt);
    yama::matrix view = cam::get_view_matrix(m_camera.obj());
    yama::matrix proj = cam::get_projection_matrix(m_camera.obj());

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
