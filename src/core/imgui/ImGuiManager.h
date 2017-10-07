#pragma once

class ImGuiManager : public Singleton<ImGuiManager>
{
public:
    ImGuiManager();
    ~ImGuiManager();

    void update(float dt);

    void onGlfwKeyEvent(int key, int action);
    void onCharEvent(unsigned c);
    
    HA_SINGLETON(ImGuiManager);
};
