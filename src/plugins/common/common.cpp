#include "core/serialization/serialization_common.h"
#include "core/serialization/serialization_2.h"
#include "core/imgui/imgui_bindings_common.h"

#include "core/Application.h"
#include "core/GraphicsHelpers.h"

#include "core/messages/messages_rendering.h"

class mesh
{
    HA_MESSAGES_IN_MIXIN(mesh);

    static void mesh_changed_cb(mesh& in) { in._mesh = MeshMan::get().get(in._path); }

    FIELD MeshHandle _mesh;
    FIELD ShaderHandle _shader;
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

        _mesh   = MeshMan::get().get(_path);
        _shader = ShaderMan::get().get("mesh");
    }

    void get_rendering_parts(std::vector<renderPart>& out) const {
        out.push_back({_mesh, {}, _shader, ha_this.get_transform().as_mat()});
    }

    AABB get_aabb() const { return getMeshBBox(_mesh.get()); }
};

HA_MIXIN_DEFINE(mesh, rend::get_rendering_parts_msg& rend::get_aabb_msg)

class parental
{
    HA_MESSAGES_IN_MIXIN(parental);
    FIELD oid m_parent;
    FIELD std::vector<oid> m_children;

    void orphan() {
        if(m_parent) {
            hassert(m_parent.obj().has<parental>());
            auto& parent_ch         = m_parent.obj().get<parental>()->m_children;
            auto  me_in_parent_iter = std::find(parent_ch.begin(), parent_ch.end(), ha_this.id());
            hassert(me_in_parent_iter != parent_ch.end());
            std::swap(*me_in_parent_iter, parent_ch.back());
            parent_ch.pop_back();
        }
    }

    void unparent() {
        while(m_children.size()) {
            auto& ch = m_children.back();
            hassert(ch);
            hassert(ch.obj().has<parental>());
            ch.obj().get<parental>()->m_parent = oid::invalid();
            m_children.pop_back();
        }
    }

public:
    HA_CLANG_SUPPRESS_WARNING("-Wdeprecated")
    ~parental() {
        if(Application::get().state() == Application::State::PLAY) {
            orphan();
            unparent();
        }
    }
    HA_CLANG_SUPPRESS_WARNING_END

    const_oid                     get_parent() const { return m_parent; }
    oid                           get_parent() { return m_parent; }
    std::vector<oid>&             get_children() { return m_children; }
    const std::vector<const_oid>& get_children() const {
        return reinterpret_cast<const std::vector<const_oid>&>(m_children);
    }

    void set_parent(oid parent) {
        orphan();
        m_parent = parent;
        if(m_parent != oid::invalid()) {
            hassert(m_parent);
            hassert(m_parent.obj().has<parental>());
            auto& parent_ch         = m_parent.obj().get<parental>()->m_children;
            auto  me_in_parent_iter = std::find(parent_ch.begin(), parent_ch.end(), ha_this.id());
            hassert(me_in_parent_iter == parent_ch.end());
            parent_ch.push_back(ha_this.id());
        }
    }
};

HA_MIXIN_DEFINE(parental, Interface_parental)

#include <gen/common.cpp.inl>
