#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif // _WIN32
#include "utils/utils.h"
#include "core/PluginManager.h"
#include "core/Application.h"

#include <bx/fpumath.h>

bgfx::ProgramHandle      mProgram;
bgfx::VertexBufferHandle mVbh;
bgfx::IndexBufferHandle  mIbh;

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

void Application::initialize(int argc, char** argv) {
    mProgram = loadProgram("shaders/glsl/vs_cubes.bin", "shaders/glsl/fs_cubes.bin");
    mVbh     = bgfx::createVertexBuffer(bgfx::makeRef(s_cubeVertices, sizeof(s_cubeVertices)),
                                    PosColorVertex::ms_decl);
    mIbh = bgfx::createIndexBuffer(bgfx::makeRef(s_cubeTriStrip, sizeof(s_cubeTriStrip)));
    bgfx::setDebug(BGFX_DEBUG_TEXT);

    m_objectManager.init();
}

void Application::update() {
    time     = (float)glfwGetTime();
    dt       = time - lastTime;
    lastTime = time;

    glfwPollEvents();
    imguiEvents(dt);
    ImGui::NewFrame();

    // update game stuff

    ImGui::ShowTestWindow(NULL);

#ifndef EMSCRIPTEN
    PluginManager::get().update();
#endif // EMSCRIPTEN
    //m_objectManager.update();

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
    bx::mtxProj(proj, 60.0f, float(getWidth()) / float(getHeight()), 0.1f, 100.0f,
                bgfx::getCaps()->homogeneousDepth);
    bgfx::setViewTransform(0, view, proj);

    // Set view 0 default viewport.
    bgfx::setViewRect(0, 0, 0, uint16_t(getWidth()), uint16_t(getHeight()));
    bgfx::touch(0);
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

    // render

    ImGui::Render();
    bgfx::frame();

    if(glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(mWindow, GL_TRUE);

    int w, h;
    glfwGetWindowSize(mWindow, &w, &h);
    if(w != mWidth || h != mHeight) {
        mWidth  = w;
        mHeight = h;
        reset(mReset);
    }
}

int main(int argc, char** argv) {
#ifndef EMSCRIPTEN
// set cwd to data folder
#ifdef _WIN32
    SetCurrentDirectory((Utils::getPathToExe() + "../../../data").c_str());
#else  // _WIN32
    chdir((Utils::getPathToExe() + "../../../data").c_str());
#endif // _WIN32
    PluginManager pluginManager;
    pluginManager.init();
#endif // EMSCRIPTEN

    doctest::Context context(argc, argv);
    int              res = context.run();

    if(context.shouldExit())
        return res;

    Application app;
    return res + app.run(argc, argv, bgfx::RendererType::OpenGL);
}
