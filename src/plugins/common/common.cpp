#include "core/registry/registry.h"
#include "core/serialization/serialization.h"
#include "core/serialization/serialization_2.h"
#include "core/imgui/imgui_stuff.h"

#include "core/Application.h"
#include "core/GraphicsHelpers.h"

#include "core/messages/messages.h"
#include "core/messages/messages_rendering.h"

class tform
{
    HA_MESSAGES_IN_MIXIN(tform);
    FIELD yama::vector3 pos    = {0, 0, 0};
    FIELD yama::vector3 scl    = {1, 1, 1};
    FIELD yama::quaternion rot = {0, 0, 0, 1};

public:
    void set_pos(const yama::vector3& in) { pos = in; }
    void set_scl(const yama::vector3& in) { scl = in; }
    void set_rot(const yama::quaternion& in) { rot = in; }

    const yama::vector3&    get_pos() const { return pos; }
    const yama::vector3&    get_scl() const { return scl; }
    const yama::quaternion& get_rot() const { return rot; }

    void set_transform(const transform& in) {
        pos = in.pos;
        scl = in.scl;
        rot = in.rot;
    }
    transform get_transform() const { return {pos, scl, rot}; }

    void move(const yama::vector3& in) { pos += in; }

    yama::matrix get_transform_mat() const {
        auto tr = yama::matrix::translation(pos);
        auto rt = yama::matrix::rotation_quaternion(rot);
        auto sc = yama::matrix::scaling(scl);
        return tr * rt * sc;
    }
};

HA_MIXIN_DEFINE(tform, Interface_transform);

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
        out.push_back({_mesh, {}, _shader, tr::get_transform_mat(ha_this)});
    }

    AABB get_aabb() const { return getMeshBBox(_mesh.get()); }
};

HA_MIXIN_DEFINE(mesh, rend::get_rendering_parts_msg& rend::get_aabb_msg);

class parental
{
    HA_MESSAGES_IN_MIXIN(parental);
    FIELD oid m_parent;
    FIELD std::vector<oid> m_children;

    void orphan() {
        if(m_parent.isValid()) {
            hassert(m_parent.get().has<parental>());
            auto& parent_ch         = m_parent.get().get<parental>()->m_children;
            auto  me_in_parent_iter = std::find(parent_ch.begin(), parent_ch.end(), ha_this.id());
            hassert(me_in_parent_iter != parent_ch.end());
            std::swap(*me_in_parent_iter, parent_ch.back());
            parent_ch.pop_back();
        }
    }

    void unparent() {
        while(m_children.size()) {
            auto& ch = m_children.back();
            hassert(ch.isValid());
            hassert(ch.get().has<parental>());
            ch.get().get<parental>()->m_parent = oid::invalid();
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

    oid                     get_parent() const { return m_parent; }
    const std::vector<oid>& get_children() const { return m_children; }

    void set_parent(oid parent) {
        orphan();
        m_parent = parent;
        if(m_parent != oid::invalid()) {
            hassert(m_parent.isValid());
            hassert(m_parent.get().has<parental>());
            auto& parent_ch         = m_parent.get().get<parental>()->m_children;
            auto  me_in_parent_iter = std::find(parent_ch.begin(), parent_ch.end(), ha_this.id());
            hassert(me_in_parent_iter == parent_ch.end());
            parent_ch.push_back(ha_this.id());
        }
    }
};

HA_MIXIN_DEFINE(parental, Interface_parental);

class selected
{
    HA_MESSAGES_IN_MIXIN(selected);
    FIELD transform old_t;

    static void submit_aabb_recursively(const Object& curr, std::vector<renderPart>& out) {
        // if object has a bbox - submit it
        if(curr.implements(rend::get_aabb_msg)) {
            auto diag   = rend::get_aabb(curr).getDiagonal();
            auto geom   = GeomMan::get().get("", createBox, diag.x, diag.y, diag.z, colors::green);
            auto shader = ShaderMan::get().get("cubes");
            out.push_back({{}, geom, shader, tr::get_transform_mat(curr)});
        }
        // recurse through children
        if(curr.implements(get_children_msg)) {
            auto& children = get_children(curr);
            for(auto& child_id : children) {
                auto& child = child_id.get();
                // if child is not selected - to avoid rendering the same bbox multiple times
                if(!child.has<selected>())
                    submit_aabb_recursively(child, out);
            }
        }
    }

public:
    void get_rendering_parts(std::vector<renderPart>& out) const {
        submit_aabb_recursively(ha_this, out);
    }

    transform& get_transform_on_gizmo_start() { return old_t; }
};

HA_MIXIN_DEFINE(selected, sel::get_transform_on_gizmo_start_msg& rend::get_rendering_parts_msg);

#include <gen/common.cpp.inl>
