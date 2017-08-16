#include "common_gen.h"

#include "core/GraphicsHelpers.h"

#include "core/messages/messages.h"
#include "core/messages/messages_rendering.h"

class transform : public transform_gen
{
    HA_MESSAGES_IN_MIXIN(transform)
public:
    void set_pos(const glm::vec3& in) { pos = in; }
    void set_scl(const glm::vec3& in) { scl = in; }
    void set_rot(const glm::quat& in) { rot = in; }

    const glm::vec3& get_pos() const { return pos; }
    const glm::vec3& get_scl() const { return scl; }
    const glm::quat& get_rot() const { return rot; }

    void move(const glm::vec3& in) { pos += in; }

    glm::mat4 get_model_transform() const {
        glm::mat4 t = glm::translate(glm::mat4(1.f), pos);
        glm::mat4 r = glm::toMat4(rot);
        return glm::scale(t * r, scl);
    }
};

HA_MIXIN_DEFINE(transform, Interface_transform);

class mesh : public mesh_gen
{
    HA_MESSAGES_IN_MIXIN(mesh)
public:
    mesh() {
        _mesh   = MeshMan::get().get("meshes/bunny.bin");
        _shader = ShaderMan::get().get("mesh");
    }

    void get_rendering_parts(std::vector<renderPart>& out) const {
        out.push_back({_mesh, {}, _shader, tr::get_model_transform(ha_this)});
    }

    AABB get_aabb() const { return getMeshBBox(_mesh.get()); }
};

HA_MIXIN_DEFINE(mesh, rend::get_rendering_parts_msg& rend::get_aabb_msg);

class hierarchical : public hierarchical_gen
{
    HA_MESSAGES_IN_MIXIN(hierarchical)
public:
    oid                     get_parent() const { return parent; }
    const std::vector<oid>& get_children() const { return children; }

    void set_parent(oid _parent) {
        hassert(parent == oid::invalid());
        parent = _parent;
    }
    void add_child(oid child) {
        hassert(std::find(children.begin(), children.end(), child) == children.end());
        children.push_back(child);
    }
    void remove_child(oid child) {
        auto it = std::find(children.begin(), children.end(), child);
        hassert(it != children.end());
        children.erase(it);
    }
};

HA_MIXIN_DEFINE(hierarchical, Interface_hierarchical);

class selected : public selected_gen
{
    HA_MESSAGES_IN_MIXIN(selected)
public:
    tinygizmo::rigid_transform& get_gizmo_transform() {
        gizmo_transform = tinygizmo::rigid_transform((const minalg::float4&)tr::get_rot(ha_this),
                                                     (const minalg::float3&)tr::get_pos(ha_this),
                                                     (const minalg::float3&)tr::get_scl(ha_this));
        return gizmo_transform;
    }

    tinygizmo::rigid_transform& get_last_stable_gizmo_transform() {
        return gizmo_transform_last;
    }

    void get_rendering_parts(std::vector<renderPart>& out) const {
        if(ha_this.implements(rend::get_aabb_msg)) {
            auto diag   = rend::get_aabb(ha_this).getDiagonal();
            auto geom   = GeomMan::get().get("", createBox, diag.x, diag.y, diag.z, colors::green);
            auto shader = ShaderMan::get().get("cubes");
            out.push_back({{}, geom, shader, tr::get_model_transform(ha_this)});
        }
    }
};

HA_MIXIN_DEFINE(selected, rend::get_rendering_parts_msg& Interface_selected);
