#include "ObjectManager.h"
#include "Application.h"

#include <iostream>

#include "core/messages/messages.h"
#include "core/messages/messages_camera.h"
#include "core/messages/messages_rendering.h"

#include "core/registry/registry.h"

HA_SUPPRESS_WARNINGS
#include <bx/fpumath.h>
HA_SUPPRESS_WARNINGS_END

#include "core/GraphicsHelpers.h"

using namespace std;

void ObjectManager::init() {
    auto& mixins = getMixins();
    for(auto& mixin : mixins)
        cout << mixin.first << endl;

    EntityManager& em = EntityManager::get();

    //Entity& object1 = em.newEntity();
    //Entity& object2 = em.newEntity();
    //Entity& object3 = em.newEntity();
    //Entity& object4 = em.newEntity();
    //Entity& object5 = em.newEntity();
    //Entity& object6 = em.newEntity();
    
    mProgram  = ShaderMan::get().get("cubes");

    editor.addMixin("editor");

    m_camera       = em.newEntityId();
    Entity& camera = m_camera.get();
    camera.addMixin("camera");
    camera.setName("camera");

    //add_child(camera, object1.id());
    //set_parent(object1, camera.id());
    //
    //add_child(camera, object2.id());
    //set_parent(object2, camera.id());
    //
    //add_child(object1, object3.id());
    //set_parent(object3, object1.id());
    //add_child(object1, object4.id());
    //set_parent(object4, object1.id());
    //add_child(object1, object5.id());
    //set_parent(object5, object1.id());
    //add_child(object1, object6.id());
    //set_parent(object6, object1.id());

    Entity& bunny = em.newEntity();
    bunny.addMixin("mesh");
    set_pos(bunny, {10, 0, 0});
    set_scl(bunny, {5, 5, 5});

    cube = GeomMan::get().get("", createCube);
    bgfx_set_debug(BGFX_DEBUG_TEXT);
}

void ObjectManager::update() {
    //auto& mixins = getMixins();
    //for(auto& mixin : mixins)
    //    if(mixin.second.update)
    //        mixin.second.update();

    //auto& globals = getGlobals();
    //for(auto& global : globals)
    //    cout << global.first << endl;

    Application& app = Application::get();
    float        dt  = app.dt();

    auto& mixins = getMixins();
    mixins["camera"].update(dt);

    bgfx_dbg_text_clear(0, false);
    bgfx_dbg_text_printf(0, 1, 0x0f, "Frame: % 7.3f[ms]", double(dt) * 1000);

    // Set view and projection matrix for view 0.

    glm::mat4 view = get_view_matrix(m_camera);
    glm::mat4 proj = get_projection_matrix(m_camera);

    bgfx_set_view_transform(0, (float*)&view, (float*)&proj);

    // Set view 0 default viewport.
    bgfx_set_view_rect(0, 0, 0, uint16(app.width()), uint16(app.height()));
    bgfx_touch(0);
    static float time = 0.f;
    time += dt;
    for(uint32 yy = 0; yy < 11; ++yy) {
        for(uint32 xx = 0; xx < 11; ++xx) {
            float mtx[16];
            bx::mtxRotateXY(mtx, time + xx * 0.21f, time + yy * 0.37f);
            mtx[12] = -15.0f + float(xx) * 3.0f;
            mtx[13] = -15.0f + float(yy) * 3.0f;
            mtx[14] = -40.0f;
            bgfx_set_transform(mtx, 1);
            bgfx_set_vertex_buffer(0, cube.get().vbh, 0, UINT32_MAX);
            bgfx_set_index_buffer(cube.get().ibh, 0, UINT32_MAX);
            bgfx_set_state(BGFX_STATE_DEFAULT | cube.get().state, 0);
            bgfx_submit(0, mProgram.get(), 0, false);
        }
    }

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

int ObjectManager::shutdown() { return 0; }
