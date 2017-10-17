#include "World.h"

#include "Application.h"

#include "core/messages/messages_camera.h"
#include "core/messages/messages_rendering.h"
#include "rendering/Renderer.h"

HA_SINGLETON_INSTANCE(World);

World::World()
        : Singleton<World>(this) {
    auto& om = ObjectManager::get();

    // load level if it exists
    auto f = fopen("level.json", "r");
    if(f) {
        fseek(f, 0, SEEK_END);
        long fsize = ftell(f);
        fseek(f, 0, SEEK_SET);
        JsonData state;
        state.reserve(fsize + 1);
        state.data().resize(fsize); // 1 less than the capacity - will add a null terminator later - which won't be included in the size
        fread(state.data().data(), fsize, 1, f);
        fclose(f);
        state.addNull();

        const auto& doc = state.parse();
        hassert(doc.is_valid());
        auto val = doc.get_root();

        auto objects_val = val.get_object_value(0);
        auto len = objects_val.get_length();
        // for each object
        for(size_t i = 0; i < len; ++i) {
            auto json_obj = objects_val.get_array_element(i);

            auto& obj = ObjectManager::get().createFromId(
                    oid(int16(json_obj.get_value_of_key({"id", 2}).get_integer_value())));

            deserialize(obj, json_obj.get_value_of_key({"state", 5}));

            auto mixins_json = json_obj.get_value_of_key({"mixins", 6});
            auto num_mixins  = mixins_json.get_length();
            for(size_t k = 0; k < num_mixins; ++k) {
                // hack to init the camera id
                auto mixin_name = mixins_json.get_object_key(k).data();
                if(strcmp(mixin_name, "gameplay_camera") == 0)
                    m_camera = obj.id();

                obj.addMixin(mixin_name);
            }

            if(obj.implements(common::deserialize_mixins_msg)) {
                common::deserialize_mixins(obj, mixins_json);
            }
        }

        return;
    }

    m_camera = om.create("camera").id();
    //m_camera.obj().addMixin("maya_camera");
    m_camera.obj().addMixin("gameplay_camera");
    m_camera.obj().addMixin("crap");

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

    mixins["gameplay_camera"].update(dt);
    yama::matrix view = cam::get_view_matrix(m_camera.obj());
    yama::matrix proj = cam::get_projection_matrix(m_camera.obj());

    auto& r = Renderer::get();

    r.setProjView(proj * view);

    auto& rd0 = r.getRenderData(0);

    for (const auto& e : ObjectManager::get().getObjects())
        if (e.second.implements(rend::get_rendering_parts_msg))
            rend::get_rendering_parts(e.second, rd0);

    mixins["editor"].update(dt);

    std::vector<const_oid*> oids;
    for(auto& obj : ObjectManager::get().getObjects())
        if(obj.second.implements(common::gather_oids_mixins_msg))
            common::gather_oids_mixins(obj.second, oids);

    printf("oids: %d\n", oids.size());
}
