#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif // EMSCRIPTEN

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif // _WIN32


#include "core/Application.h"
#include "core/PluginManager.h"
#include "utils/utils.h"
#include "utils/preprocessor.h"

// TODO: use a smarter allocator - the important methods here are for the mixin data
class global_mixin_allocator : public dynamix::global_allocator
{
    char* alloc_mixin_data(size_t count) override { return new char[count * mixin_data_size]; }
    void dealloc_mixin_data(char* ptr) override { delete[] ptr; }

    void alloc_mixin(size_t mixin_size, size_t mixin_alignment, char*& out_buffer,
                     size_t& out_mixin_offset) override {
        const size_t size = calculate_mem_size_for_mixin(mixin_size, mixin_alignment);
        out_buffer        = new char[size];
        out_mixin_offset  = calculate_mixin_offset(out_buffer, mixin_alignment);
    }
    void dealloc_mixin(char* ptr) override { delete[] ptr; }
};

//#include "ocornut-imgui/imgui.h"
#include <bx/uint32_t.h>
#include "common.h"
#include "entry/entry_p.h"
#include "bgfx_utils.h"
#include "logo.h"

uint32_t                 m_width;
uint32_t                 m_height;
uint32_t                 m_debug;
uint32_t                 m_reset;
bgfx::VertexBufferHandle m_vbh;
bgfx::IndexBufferHandle  m_ibh;
bgfx::ProgramHandle      m_program;
int64_t                  m_timeOffset;

struct PosColorVertex
{
    float    m_x;
    float    m_y;
    float    m_z;
    uint32_t m_abgr;

    static void init() {
        ms_decl.begin()
                .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
                .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
                .end();
    };

    static bgfx::VertexDecl ms_decl;
};

bgfx::VertexDecl PosColorVertex::ms_decl;

static PosColorVertex s_cubeVertices[] = {
        {-1.0f, 1.0f, 1.0f, 0xff000000},   {1.0f, 1.0f, 1.0f, 0xff0000ff},
        {-1.0f, -1.0f, 1.0f, 0xff00ff00},  {1.0f, -1.0f, 1.0f, 0xff00ffff},
        {-1.0f, 1.0f, -1.0f, 0xffff0000},  {1.0f, 1.0f, -1.0f, 0xffff00ff},
        {-1.0f, -1.0f, -1.0f, 0xffffff00}, {1.0f, -1.0f, -1.0f, 0xffffffff},
};

static const uint16_t s_cubeTriList[] = {
        0, 1, 2,          // 0
        1, 3, 2, 4, 6, 5, // 2
        5, 6, 7, 0, 2, 4, // 4
        4, 2, 6, 1, 5, 3, // 6
        5, 7, 3, 0, 4, 1, // 8
        4, 5, 1, 2, 3, 6, // 10
        6, 3, 7,
};

static const uint16_t s_cubeTriStrip[] = {
        0, 1, 2, 3, 7, 1, 5, 0, 4, 2, 6, 7, 4, 5,
};

bool update();

#ifdef EMSCRIPTEN
static void em_update() { update(); }
#endif // EMSCRIPTEN

int _main_(int _argc, char** _argv) {
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

    doctest::Context context(_argc, _argv);
    int              res = context.run();

    if(context.shouldExit())
        return res;

    global_mixin_allocator alloc;
    Application            app;
    Args                   args(_argc, _argv);

    dynamix::set_global_allocator(&alloc);

    app.init();

    m_width  = 1280;
    m_height = 720;
    m_debug  = BGFX_DEBUG_TEXT;
    m_reset  = BGFX_RESET_VSYNC;

    bgfx::init(args.m_type, args.m_pciId);
    bgfx::reset(m_width, m_height, m_reset);

    // Enable debug text.
    bgfx::setDebug(m_debug);

    // Set view 0 clear state.
    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);

    // Create vertex stream declaration.
    PosColorVertex::init();

    // Create static vertex buffer.
    m_vbh = bgfx::createVertexBuffer(
            // Static data can be passed with bgfx::makeRef
            bgfx::makeRef(s_cubeVertices, sizeof(s_cubeVertices)), PosColorVertex::ms_decl);

    // Create static index buffer.
    m_ibh = bgfx::createIndexBuffer(
            // Static data can be passed with bgfx::makeRef
            bgfx::makeRef(s_cubeTriStrip, sizeof(s_cubeTriStrip)));

    // Create program from shaders.
    m_program = loadProgram("vs_cubes", "fs_cubes");

    m_timeOffset = bx::getHPCounter();

    // after init we draw the first frame

    bgfx::frame();

    entry::WindowHandle defaultWindow = {0};
    entry::setWindowSize(defaultWindow, ENTRY_DEFAULT_WIDTH, ENTRY_DEFAULT_HEIGHT);

#if BX_PLATFORM_EMSCRIPTEN
    emscripten_set_main_loop(&em_update, -1, 1);
#else
    while(update())
        ;
#endif // BX_PLATFORM_EMSCRIPTEN

    // Cleanup.
    bgfx::destroyIndexBuffer(m_ibh);
    bgfx::destroyVertexBuffer(m_vbh);
    bgfx::destroyProgram(m_program);

    bgfx::shutdown();

    return res;
}

bool update() {
    if(!entry::processEvents(m_width, m_height, m_debug, m_reset)) {
        //ImGui::ShowTestWindow();

        Application::get().update();

        int64_t        now       = bx::getHPCounter();
        static int64_t last      = now;
        const int64_t  frameTime = now - last;
        last                     = now;
        const double freq        = double(bx::getHPFrequency());
        const double toMs        = 1000.0 / freq;

        float time = (float)((now - m_timeOffset) / double(bx::getHPFrequency()));

        // Use debug font to print information about this example.
        bgfx::dbgTextClear();
        bgfx::dbgTextPrintf(0, 1, 0x4f, "bgfx/examples/01-cube");
        bgfx::dbgTextPrintf(0, 2, 0x6f, "Description: Rendering simple static mesh.");
        bgfx::dbgTextPrintf(0, 3, 0x0f, "Frame: % 7.3f[ms]", double(frameTime) * toMs);

        float at[3]  = {0.0f, 0.0f, 0.0f};
        float eye[3] = {0.0f, 0.0f, -35.0f};

        // Set view and projection matrix for view 0.
        const bgfx::HMD* hmd = bgfx::getHMD();
        if(NULL != hmd && 0 != (hmd->flags & BGFX_HMD_RENDERING)) {
            float view[16];
            bx::mtxQuatTranslationHMD(view, hmd->eye[0].rotation, eye);
            bgfx::setViewTransform(0, view, hmd->eye[0].projection, BGFX_VIEW_STEREO,
                                   hmd->eye[1].projection);

            // Set view 0 default viewport.
            //
            // Use HMD's width/height since HMD's internal frame buffer size
            // might be much larger than window size.
            bgfx::setViewRect(0, 0, 0, hmd->width, hmd->height);
        } else {
            float view[16];
            bx::mtxLookAt(view, eye, at);

            float proj[16];
            bx::mtxProj(proj, 60.0f, float(m_width) / float(m_height), 0.1f, 100.0f,
                        bgfx::getCaps()->homogeneousDepth);
            bgfx::setViewTransform(0, view, proj);

            // Set view 0 default viewport.
            bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height));
        }

        // This dummy draw call is here to make sure that view 0 is cleared
        // if no other draw calls are submitted to view 0.
        bgfx::touch(0);

        // Submit 11x11 cubes.
        for(uint32_t yy = 0; yy < 11; ++yy) {
            for(uint32_t xx = 0; xx < 11; ++xx) {
                float mtx[16];
                bx::mtxRotateXY(mtx, time + xx * 0.21f, time + yy * 0.37f);
                mtx[12] = -15.0f + float(xx) * 3.0f;
                mtx[13] = -15.0f + float(yy) * 3.0f;
                mtx[14] = 0.0f;

                // Set model matrix for rendering.
                bgfx::setTransform(mtx);

                // Set vertex and index buffer.
                bgfx::setVertexBuffer(0, m_vbh);
                bgfx::setIndexBuffer(m_ibh);

                // Set render states.
                bgfx::setState(0 | BGFX_STATE_DEFAULT | BGFX_STATE_PT_TRISTRIP);

                // Submit primitive for rendering to view 0.
                bgfx::submit(0, m_program);
            }
        }

        // Advance to next frame. Rendering thread will be kicked to
        // process submitted rendering primitives.
        bgfx::frame();

        //// Set view 0 default viewport.
        //bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height));

        //// This dummy draw call is here to make sure that view 0 is cleared
        //// if no other draw calls are submitted to view 0.
        //bgfx::touch(0);

        //// Use debug font to print information about this example.
        //bgfx::dbgTextClear();
        //bgfx::dbgTextImage(bx::uint16_max(uint16_t(m_width / 2 / 8), 20) - 20,
        //                   bx::uint16_max(uint16_t(m_height / 2 / 16), 6) - 6, 40, 12, s_logo, 160);
        //bgfx::dbgTextPrintf(0, 1, 0x4f, "bgfx/examples/00-helloworld");
        //bgfx::dbgTextPrintf(0, 2, 0x6f, "Description: Initialization and debug text.");

        //bgfx::dbgTextPrintf(0, 4, 0x0f, "Color can be changed with ANSI "
        //                                "\x1b[9;me\x1b[10;ms\x1b[11;mc\x1b[12;ma\x1b[13;mp\x1b["
        //                                "14;me\x1b[0m code too.");

        //const bgfx::Stats* stats = bgfx::getStats();
        //bgfx::dbgTextPrintf(0, 6, 0x0f,
        //                    "Backbuffer %dW x %dH in pixels, debug text %dW x %dH in characters.",
        //                    stats->width, stats->height, stats->textWidth, stats->textHeight);

        //// Advance to next frame. Rendering thread will be kicked to
        //// process submitted rendering primitives.
        //bgfx::frame();

        return true;
    }

    return false;
}
