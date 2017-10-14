#include "core/serialization/serialization_common.h"
#include "core/serialization/serialization_2.h"
#include "core/imgui/imgui_bindings_common.h"

#include "core/rendering/GraphicsHelpers.h"

#include "core/messages/messages_rendering.h"

class mesh
{
    HA_MESSAGES_IN_MIXIN(mesh);

    static void mesh_changed_cb(mesh&) {}

    GeomMan::Handle _mesh;

    REFL_ATTRIBUTES(tag::mesh, REFL_CALLBACK(mesh::mesh_changed_cb))
    FIELD std::string _path;
    REFL_ATTRIBUTES(tag::image)
    FIELD std::string _image_path;

    FIELD bool   clicky      = false;
    FIELD float  dragy       = 42;
    FIELD double dragy2      = 42;
    FIELD int    dragy3      = 42;
    FIELD std::string texty  = "happy!!";
    FIELD std::string texty2 = ":(";

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

REFL_ATTRIBUTES(REFL_NO_SKIP)
class crap
{
    HA_MESSAGES_IN_MIXIN(crap);
};

HA_MIXIN_DEFINE(crap, dynamix::none)

#include <gen/common.cpp.inl>
