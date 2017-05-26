#include "Application.h"
#include "PluginManager.h"

using namespace std;

HARDLY_SCOPED_SINGLETON_IMPLEMENT(Application);

void Application::init() {
    m_objectManager.init();
}

void Application::update() {
#ifndef EMSCRIPTEN
    PluginManager::get().update();
#endif // EMSCRIPTEN
    m_objectManager.update();
}
