#include "core/serialization/serialization_common.h"
#include "core/serialization/serialization_2.h"
#include "core/imgui/imgui_bindings_common.h"

#include "core/rendering/GraphicsHelpers.h"

#include "core/messages/messages_rendering.h"

//class crap
//{
//    HA_MESSAGES_IN_MIXIN(crap);
//};
//
//HA_MIXIN_DEFINE(crap, dynamix::none)

class mesh
{
    HA_MESSAGES_IN_MIXIN(mesh);

    static void mesh_changed_cb(mesh&) {}

    ATTRS(skip) GeomMan::Handle _mesh;

    //REFL_ATTRIBUTES(tag::mesh, REFL_CALLBACK(mesh::mesh_changed_cb))
    std::string _path;
    //REFL_ATTRIBUTES(tag::image)
    std::string _image_path;

    bool   clicky      = false;
    float  dragy       = 42;
    double dragy2      = 42;
    int    dragy3      = 42;
    std::string texty  = "happy!!";
    std::string texty2 = ":(";
    oid some_obj_id;
    std::vector<oid> omg;
    std::map<int, oid> omg2;

public:
    mesh() {
        _path = "meshes/bunny.bin";

        _mesh = GeomMan::get().get("", createSolidBox, 1.f, 1.f, 1.f, 0xFFFFFFFF);
    }

    void get_rendering_parts(std::vector<renderPart>& out) const {
        out.push_back({_mesh, TempMesh(), ha_this.get_transform().as_mat()});
    }

    AABB get_aabb() const { return _mesh.get().bbox; }
};

HA_MIXIN_DEFINE(mesh, rend::get_rendering_parts_msg& rend::get_aabb_msg)

#include <gen/common.cpp.inl>
