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

HA_SCOPED_SINGLETON_IMPLEMENT(ObjectManager);

static bgfx::ProgramHandle      mProgram;
static bgfx::VertexBufferHandle mVbh;
static bgfx::IndexBufferHandle  mIbh;

// vertex declarations
struct PosColorVertex
{
    float                   x;
    float                   y;
    float                   z;
    uint32                  abgr;
    static void             init();
    static bgfx::VertexDecl ms_decl;
};

void PosColorVertex::init() {
    ms_decl.begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
            .end();
}

bgfx::VertexDecl PosColorVertex::ms_decl;

static PosColorVertex s_cubeVertices[] = {
        {-1.0f, 1.0f, 1.0f, 0xff000000},   {1.0f, 1.0f, 1.0f, 0xff0000ff},
        {-1.0f, -1.0f, 1.0f, 0xff00ff00},  {1.0f, -1.0f, 1.0f, 0xff00ffff},
        {-1.0f, 1.0f, -1.0f, 0xffff0000},  {1.0f, 1.0f, -1.0f, 0xffff00ff},
        {-1.0f, -1.0f, -1.0f, 0xffffff00}, {1.0f, -1.0f, -1.0f, 0xffffffff},
};
static const uint16 s_cubeTriStrip[] = {
        0, 1, 2, 3, 7, 1, 5, 0, 4, 2, 6, 7, 4, 5,
};

void ObjectManager::init() {
    auto& mixins = getMixins();
    for(auto& mixin : mixins)
        cout << mixin.first << endl;

    Entity& object1 = newObject();
    Entity& object2 = newObject();
    Entity& object3 = newObject();
    Entity& object4 = newObject();
    Entity& object5 = newObject();
    Entity& object6 = newObject();

    m_camera       = newObjectId();
    Entity& camera = getObject(m_camera);
    addMixin(camera, "camera");
    camera.setName("camera");

    add_child(camera, object1.id());
    set_parent(object1, camera.id());

    add_child(camera, object2.id());
    set_parent(object2, camera.id());

    add_child(object1, object3.id());
    set_parent(object3, object1.id());
    add_child(object1, object4.id());
    set_parent(object4, object1.id());
    add_child(object1, object5.id());
    set_parent(object5, object1.id());
    add_child(object1, object6.id());
    set_parent(object6, object1.id());

    //Entity& bunny = newObject();
    //addMixin(bunny, "mesh");
    //set_scl(bunny, glm::vec3(20, 20, 20));

    // Setup vertex declarations
    PosColorVertex::init();

    mProgram = loadProgram("cubes_vs", "cubes_fs");
    mVbh     = bgfx::createVertexBuffer(bgfx::makeRef(s_cubeVertices, sizeof(s_cubeVertices)),
                                    PosColorVertex::ms_decl);
    mIbh = bgfx::createIndexBuffer(bgfx::makeRef(s_cubeTriStrip, sizeof(s_cubeTriStrip)));
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

    static bool no_titlebar  = false;
    static bool no_border    = true;
    static bool no_resize    = false;
    static bool no_move      = false;
    static bool no_scrollbar = false;
    static bool no_collapse  = false;
    static bool no_menu      = true;

    // Demonstrate the various window flags. Typically you would just use the default.
    ImGuiWindowFlags window_flags = 0;
    // clang-format off
    if (no_titlebar)  window_flags |= ImGuiWindowFlags_NoTitleBar;
    if (!no_border)   window_flags |= ImGuiWindowFlags_ShowBorders;
    if (no_resize)    window_flags |= ImGuiWindowFlags_NoResize;
    if (no_move)      window_flags |= ImGuiWindowFlags_NoMove;
    if (no_scrollbar) window_flags |= ImGuiWindowFlags_NoScrollbar;
    if (no_collapse)  window_flags |= ImGuiWindowFlags_NoCollapse;
    if (!no_menu)     window_flags |= ImGuiWindowFlags_MenuBar;
    ImGui::SetNextWindowSize(ImVec2(400,600), ImGuiSetCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(0, 100), ImGuiSetCond_FirstUseEver);
    // clang-format on

    static vector<eid> selected;

    if(ImGui::Begin("scene explorer", nullptr, window_flags)) {
        if(ImGui::TreeNode("objects")) {
            //auto hierarchical_id = dynamix::internal::domain::instance().get_mixin_id_by_name("hierarchical");

            for(const auto& curr : m_objects) {
                // recursive select/deselect
                std::function<void(eid, bool)> recursiveSelecter = [&](eid root, bool select) {
                    auto& obj = getObject(root);
                    auto  it  = std::find(selected.begin(), selected.end(), root);
                    if(select) {
                        if(it == selected.end())
                            selected.push_back(root);
                    } else {
                        if(it != selected.end())
                            selected.erase(it);
                    }
                    obj.select(select);

                    // recurse through children
                    const auto& children = ::get_children(obj);
                    if(children.size() > 0)
                        for(const auto& c : children)
                            recursiveSelecter(c, select);
                };

                // recursive tree build
                std::function<void(eid)> buildTree = [&](eid root) {
                    auto&              obj = getObject(root);
                    ImGuiTreeNodeFlags node_flags =
                            ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
                            (obj.selected() ? ImGuiTreeNodeFlags_Selected : 0);

                    const auto& children = ::get_children(obj);
                    if(children.size() == 0)
                        node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

                    auto name      = obj.name() + " (" + std::to_string(int(obj.id())) + ")";
                    bool node_open = ImGui::TreeNodeEx((void*)(intptr_t) int(root), node_flags,
                                                       name.c_str());

                    if(ImGui::IsItemClicked()) {
                        bool shouldSelect = !obj.selected();

                        if(ImGui::GetIO().KeyShift) {
                            recursiveSelecter(root, shouldSelect);
                        } else if(ImGui::GetIO().KeyCtrl) {
                            obj.select(shouldSelect);
                            if(shouldSelect)
                                selected.push_back(root);
                            else
                                selected.erase(std::find(selected.begin(), selected.end(), root));
                        } else if(!obj.selected()) {
                            for(auto& it : selected) {
                                getObject(it).select(false);
                            }
                            selected.clear();
                            obj.select(shouldSelect);
                            selected.push_back(root);
                        }
                    }

                    if(node_open && children.size() > 0) {
                        for(const auto& c : children)
                            buildTree(c);
                        ImGui::TreePop();
                    }
                };

                // recurse from those without a parent only
                if(curr.second.implements(get_parent_msg)) {
                    if(::get_parent(curr.second) == eid::invalid())
                        buildTree(curr.second.id());
                }
            }
            ImGui::TreePop();
        }
    }
    ImGui::End();

    ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiSetCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(float(Application::get().width() - 400), 0),
                            ImGuiSetCond_FirstUseEver);

    if(ImGui::Begin("object properties", nullptr, window_flags)) {
        for(auto& id : selected) {
            auto& obj = getObject(id);
            if(ImGui::TreeNode(obj.name().c_str())) {
                if(obj.implements(imgui_bind_properties_msg)) {
                    imgui_bind_properties(obj);
                }

                ImGui::TreePop();
            }
        }
    }
    ImGui::End();

    //ImGui::ShowTestWindow();

    Application& app = Application::get();
    float        dt  = app.dt();

    auto& mixins = getMixins();
    mixins["camera"].update(dt);

    bgfx::dbgTextClear();
    bgfx::dbgTextPrintf(0, 1, 0x4f, "bgfx/examples/01-cube");
    bgfx::dbgTextPrintf(0, 2, 0x6f, "Description: Rendering simple static mesh.");
    bgfx::dbgTextPrintf(0, 3, 0x0f, "Frame: % 7.3f[ms]", double(dt) * 1000);

    // Set view and projection matrix for view 0.

    glm::mat4 view = get_view_matrix(getObject(m_camera));
    glm::mat4 proj = get_projection_matrix(getObject(m_camera));

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
            bgfx::setVertexBuffer(0, mVbh);
            bgfx::setIndexBuffer(mIbh);
            bgfx::setState(BGFX_STATE_DEFAULT | BGFX_STATE_PT_TRISTRIP);
            bgfx::submit(0, mProgram);
        }
    }

    std::vector<renderPart> renderData;

    for(const auto& obj : m_objects)
        if(obj.second.implements(get_rendering_parts_msg))
            get_rendering_parts(obj.second, renderData);
    for(const auto& data : renderData)
        meshSubmit(data.mesh.get(), 0, data.shader.get(), (float*)&data.transform);
}

int ObjectManager::shutdown() {
    bgfx::destroyIndexBuffer(mIbh);
    bgfx::destroyVertexBuffer(mVbh);
    bgfx::destroyProgram(mProgram);
    return 0;
}

eid ObjectManager::newObjectId(const std::string& in_name) {
    std::string name = in_name;
    if(name.empty())
        name = "object_" + std::to_string(m_curr_id);

    auto it = m_objects.emplace(eid(m_curr_id), Entity(eid(m_curr_id), name));
    addMixin(it.first->second, "transform");
    addMixin(it.first->second, "hierarchical");
    return eid(m_curr_id++);
}

Entity& ObjectManager::newObject(const std::string& in_name) {
    return getObject(newObjectId(in_name));
}

Entity& ObjectManager::getObject(eid id) { return m_objects[id]; }

void ObjectManager::addMixin(Entity& obj, const char* mixin) {
    auto& mixins = getMixins();
    hassert(mixins.find(mixin) != mixins.end());
    mixins[mixin].add(&obj);
}

void ObjectManager::remMixin(Entity& obj, const char* mixin) {
    auto& mixins = getMixins();
    hassert(mixins.find(mixin) != mixins.end());
    mixins[mixin].remove(&obj);
}
