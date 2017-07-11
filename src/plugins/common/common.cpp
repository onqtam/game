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
        out.push_back({_mesh, _shader, get_model_transform(ha_this)});
    }
};

HA_MIXIN_DEFINE(mesh, get_rendering_parts_msg);

class hierarchical : public hierarchical_gen
{
    HA_MESSAGES_IN_MIXIN(hierarchical)
public:
    eid                     get_parent() const { return parent; }
    const std::vector<eid>& get_children() const { return children; }

    void set_parent(eid _parent) {
        hassert(parent == eid::invalid());
        parent = _parent;
    }
    void add_child(eid child) {
        hassert(std::find(children.begin(), children.end(), child) == children.end());
        children.push_back(child);
    }
    void remove_child(eid child) {
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
        gizmo_transform = tinygizmo::rigid_transform((minalg::float4&)get_rot(ha_this),
                                                     (minalg::float3&)get_pos(ha_this),
                                                     (minalg::float3&)get_scl(ha_this));
        return gizmo_transform;
    }

    void get_rendering_parts(std::vector<renderPart>&) const {
        //out.push_back({_mesh, _shader, get_model_transform(ha_this)});
    }
};

HA_MIXIN_DEFINE(selected, get_rendering_parts_msg& get_gizmo_transform_msg);