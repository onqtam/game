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
    
    mProgram = ShaderMan::get().get("cubes");
    asd = GeomMan::get().get("cube");
    bgfx::setDebug(BGFX_DEBUG_TEXT);
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

    bgfx::dbgTextClear();
    bgfx::dbgTextPrintf(0, 1, 0x0f, "Frame: % 7.3f[ms]", double(dt) * 1000);

    // Set view and projection matrix for view 0.

    glm::mat4 view = get_view_matrix(m_camera);
    glm::mat4 proj = get_projection_matrix(m_camera);

    bgfx::setViewTransform(0, (float*)&view, (float*)&proj);

    // Set view 0 default viewport.
    bgfx::setViewRect(0, 0, 0, uint16(app.width()), uint16(app.height()));
    bgfx::touch(0);
    static float time = 0.f;
    time += dt;
    for(uint32 yy = 0; yy < 11; ++yy) {
        for(uint32 xx = 0; xx < 11; ++xx) {
            float mtx[16];
            bx::mtxRotateXY(mtx, time + xx * 0.21f, time + yy * 0.37f);
            mtx[12] = -15.0f + float(xx) * 3.0f;
            mtx[13] = -15.0f + float(yy) * 3.0f;
            mtx[14] = -40.0f;
            bgfx::setTransform(mtx);
            bgfx::setVertexBuffer(0, asd.get().vbh);
            bgfx::setIndexBuffer(asd.get().ibh);
            bgfx::setState(BGFX_STATE_DEFAULT | BGFX_STATE_PT_TRISTRIP /*| BGFX_STATE_PT_LINES*/);
            bgfx::submit(0, mProgram.get());
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

int ObjectManager::shutdown() {
    return 0;
}
