#ifdef EMSCRIPTEN
#include <emscripten/emscripten.h>
#endif // EMSCRIPTEN

#include "core/Application.h"
#include "core/PluginManager.h"
#include "utils/utils.h"
#include "utils/preprocessor.h"

HARDLY_SUPPRESS_WARNINGS
#include <GL/glew.h>
#if defined(_WIN32)
#include <GL/wglew.h>
#elif !defined(EMSCRIPTEN) && (!defined(__APPLE__) || defined(GLEW_APPLE_GLX))
#include <GL/glxew.h>
#endif

#include <GLFW/glfw3.h>
HARDLY_SUPPRESS_WARNINGS_END

#include <bgfx/bgfx.h>

static GLFWwindow* window;

static void errorCallback(int error, const char* description) {
    fprintf(stderr, "%s\nWith error: %d\n", description, error);
}

static void renderGame() {
    glClear(GL_COLOR_BUFFER_BIT);

    double ratio;
    int    width, height;

    glfwGetFramebufferSize(window, &width, &height);
    ratio = width / double(height);
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-ratio, ratio, -1., 1., 1., -1.);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(float(glfwGetTime()) * 50.f, 0.f, 0.f, 1.f);
    glBegin(GL_TRIANGLES);
    glColor3f(1.f, 0.f, 0.f);
    glVertex3f(-0.6f, -0.4f, 0.f);
    glColor3f(0.f, 1.f, 0.f);
    glVertex3f(0.6f, -0.4f, 0.f);
    glColor3f(0.f, 0.f, 1.f);
    glVertex3f(0.f, 0.6f, 0.f);
    glEnd();

    Application::get().update();

#ifdef EMSCRIPTEN
    glfwSwapBuffers(window);
    glfwPollEvents();

    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if(glfwWindowShouldClose(window))
        glfwTerminate();
#endif // EMSCRIPTEN
}

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

int main(int argc, char** argv) {
#ifndef EMSCRIPTEN
    PluginManager pluginManager;
    pluginManager.init();
#endif // EMSCRIPTEN

    doctest::Context context(argc, argv);
    int              res = context.run();

    if(context.shouldExit())
        return res;

    glfwSetErrorCallback(errorCallback);

    if(!glfwInit()) {
        fputs("Failed to initialize GLFW3!", stderr);
        exit(EXIT_FAILURE);
    }

    window = glfwCreateWindow(640, 480, "Hardly game", nullptr, nullptr);

    if(!window) {
        fputs("Failed to create GLFW3 window!", stderr);
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);

    GLenum err = glewInit();
    if(GLEW_OK != err) {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glClearColor(0.0, 0.0, 0.0, 1.0);

    global_mixin_allocator alloc;
    dynamix::set_global_allocator(&alloc);

    Application app;
    app.init();

#ifdef EMSCRIPTEN
    emscripten_set_main_loop(renderGame, 0, 1);
#else  // EMSCRIPTEN
    while(!glfwWindowShouldClose(window)) {
        glfwSwapBuffers(window);
        glfwPollEvents();

        renderGame();

        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GL_TRUE);
    }
#endif // EMSCRIPTEN

    return res;
}
