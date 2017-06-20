#include "ObjectManager.h"
#include "Application.h"

#include <iostream>

#include "mixins/messages/messages.h"

#include "core/registry/registry.h"
#include "serialization/JsonData.h"

#include <bgfx/bgfx.h>
#include <bx/fpumath.h>
#include <imgui.h>
#include "core/GraphicsHelpers.h"

using namespace std;

HARDLY_SCOPED_SINGLETON_IMPLEMENT(ObjectManager);

bgfx::ProgramHandle      mProgram;
bgfx::VertexBufferHandle mVbh;
bgfx::IndexBufferHandle  mIbh;

Mesh*               m_mesh;
bgfx::ProgramHandle m_program;

// vertex declarations
struct PosColorVertex
{
    float                   x;
    float                   y;
    float                   z;
    uint32_t                abgr;
    static void             init();
    static bgfx::VertexDecl ms_decl;
};

void PosColorVertex::init() {
    ms_decl.begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
            .end();
};

bgfx::VertexDecl PosColorVertex::ms_decl;

static PosColorVertex s_cubeVertices[] = {
        {-1.0f, 1.0f, 1.0f, 0xff000000},   {1.0f, 1.0f, 1.0f, 0xff0000ff},
        {-1.0f, -1.0f, 1.0f, 0xff00ff00},  {1.0f, -1.0f, 1.0f, 0xff00ffff},
        {-1.0f, 1.0f, -1.0f, 0xffff0000},  {1.0f, 1.0f, -1.0f, 0xffff00ff},
        {-1.0f, -1.0f, -1.0f, 0xffffff00}, {1.0f, -1.0f, -1.0f, 0xffffffff},
};
static const uint16_t s_cubeTriList[] = {
        0, 1, 2, 1, 3, 2, 4, 6, 5, 5, 6, 7, 0, 2, 4, 4, 2, 6,
        1, 5, 3, 5, 7, 3, 0, 4, 1, 4, 5, 1, 2, 3, 6, 6, 3, 7,
};
static const uint16_t s_cubeTriStrip[] = {
        0, 1, 2, 3, 7, 1, 5, 0, 4, 2, 6, 7, 4, 5,
};

void ObjectManager::init() {
    auto& mixins = getMixins();
    for(auto& mixin : mixins)
        cout << mixin.first << endl;

    dynamix::object& m_object = get_object(new_object());
    addMixin(m_object, "dummy");

    m_camera = new_object();
    addMixin(get_object(m_camera), "camera");


    //dynamix::object o1, o2, o3, o4;

    //addMixin(o1, "dummy");
    //addMixin(o2, "dummy");
    //addMixin(o3, "dummy");
    //remMixin(o1, "dummy");
    //addMixin(o4, "dummy");

    //addMixin(m_object, "trololo");

    //JsonData state;
    //state.reserve(1000);
    //state.startObject();
    //serialize(m_object, state);
    //state.endObject();

    //const sajson::document& doc = state.parse();
    //PPK_ASSERT(doc.is_valid());
    //const sajson::value root = doc.get_root();
    //deserialize(m_object, root);

    // Setup vertex declarations
    PosColorVertex::init();

    mProgram = loadProgram("cubes_vs", "cubes_fs");
    mVbh     = bgfx::createVertexBuffer(bgfx::makeRef(s_cubeVertices, sizeof(s_cubeVertices)),
                                    PosColorVertex::ms_decl);
    mIbh = bgfx::createIndexBuffer(bgfx::makeRef(s_cubeTriStrip, sizeof(s_cubeTriStrip)));
    bgfx::setDebug(BGFX_DEBUG_TEXT);

    m_mesh    = meshLoad("meshes/bunny.bin");
    m_program = loadProgram("mesh_vs", "mesh_fs");
}

void ObjectManager::update() {
    //cout << " ====== trace ====== " << endl;
    //trace(m_object, cout);

    //auto& mixins = getMixins();
    //for(auto& mixin : mixins)
    //    if(mixin.second.update)
    //        mixin.second.update();

    //auto& globals = getGlobals();
    //for(auto& global : globals)
    //    cout << global.first << endl;

    ImGui::ShowTestWindow();

    Application& app = Application::get();
    float        dt  = app.getDt();

    bgfx::dbgTextClear();
    bgfx::dbgTextPrintf(0, 1, 0x4f, "bgfx/examples/01-cube");
    ;
    bgfx::dbgTextPrintf(0, 2, 0x6f, "Description: Rendering simple static mesh.");
    bgfx::dbgTextPrintf(0, 3, 0x0f, "Frame: % 7.3f[ms]", double(dt) * 1000);

    float at[3]  = {0.0f, 0.0f, 0.0f};
    float eye[3] = {0.0f, 0.0f, -35.0f};

    // Set view and projection matrix for view 0.
    float view[16];
    bx::mtxLookAt(view, eye, at);
    float proj[16];
    bx::mtxProj(proj, 60.0f, float(app.getWidth()) / float(app.getHeight()), 0.1f, 100.0f,
                bgfx::getCaps()->homogeneousDepth);
    bgfx::setViewTransform(0, view, proj);

    // Set view 0 default viewport.
    bgfx::setViewRect(0, 0, 0, uint16_t(app.getWidth()), uint16_t(app.getHeight()));
    bgfx::touch(0);
    static float time = 0.f;
    time += dt;
    for(uint32_t yy = 0; yy < 11; ++yy) {
        for(uint32_t xx = 0; xx < 11; ++xx) {
            float mtx[16];
            bx::mtxRotateXY(mtx, time + xx * 0.21f, time + yy * 0.37f);
            mtx[12] = -15.0f + float(xx) * 3.0f;
            mtx[13] = -15.0f + float(yy) * 3.0f;
            mtx[14] = 0.0f;
            bgfx::setTransform(mtx);
            bgfx::setVertexBuffer(0, mVbh);
            bgfx::setIndexBuffer(mIbh);
            bgfx::setState(BGFX_STATE_DEFAULT | BGFX_STATE_PT_TRISTRIP);
            bgfx::submit(0, mProgram);
        }
    }

    float mtx1[16];
    bx::mtxRotateXY(mtx1, 0.0f, time * 0.37f);

    float mtx2[16];
    bx::mtxScale(mtx2, 10.0f);

    float mtx[16];
    bx::mtxMul(mtx, mtx2, mtx1);

    meshSubmit(m_mesh, 0, m_program, mtx);
}

int ObjectManager::shutdown() {
    bgfx::destroyProgram(m_program);
    meshUnload(m_mesh);

    bgfx::destroyIndexBuffer(mIbh);
    bgfx::destroyVertexBuffer(mVbh);
    bgfx::destroyProgram(mProgram);
    return 0;
}

int ObjectManager::new_object() {
    auto it = m_objects.emplace(m_curr_id, dynamix::object());
    addMixin(it.first->second, "common");
    set_id(it.first->second, m_curr_id);
    return m_curr_id++;
}

dynamix::object& ObjectManager::get_object(int id) {
    return m_objects[id];
}

void ObjectManager::addMixin(dynamix::object& obj, const char* mixin) {
    auto& mixins = getMixins();
    PPK_ASSERT(mixins.find(mixin) != mixins.end());
    mixins[mixin].add(&obj);
}

void ObjectManager::remMixin(dynamix::object& obj, const char* mixin) {
    auto& mixins = getMixins();
    PPK_ASSERT(mixins.find(mixin) != mixins.end());
    mixins[mixin].remove(&obj);
}
