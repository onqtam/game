#include "core/registry/registry.h"
#include "core/serialization/serialization.h"
#include "core/serialization/serialization_2.h"
#include "core/imgui/imgui_stuff.h"

#include "core/GraphicsHelpers.h"

#include "core/messages/messages.h"
#include "core/messages/messages_rendering.h"

class camera;
void        serialize(const camera& src, JsonData& out, bool as_object);
size_t      deserialize(camera& dest, const sajson::value& val);
const char* imgui_bind_attributes(Object& e, const char* mixin_name, camera& obj);

class transform
{
    HA_FRIENDS_OF_TYPE(transform);
    HA_MESSAGES_IN_MIXIN(transform)
    FIELD glm::vec3 pos = {0, 0, 0};
    FIELD glm::vec3 scl = {1, 1, 1};
    FIELD glm::quat rot = {1, 0, 0, 0};

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

class mesh
{
    HA_FRIENDS_OF_TYPE(mesh);
    FIELD mesh_path _path;
    FIELD MeshHandle _mesh;
    FIELD ShaderHandle _shader;

public:
    std::map<std::string, std::vector<std::function<void(void)>>> attr_changed_callbacks;

    void serialize_mixins(JsonData& out) const {
        out.append("\"mesh\":");
        serialize(*this, out);
        out.addComma();
    }
    void deserialize_mixins(const sajson::value& in) {
        auto str = sajson::string("mesh", HA_COUNT_OF("mesh") - 1);
        if(in.find_object_key(str) != in.get_length())
            deserialize(*this, in.get_value_of_key(str));
    }
    void set_attribute_mixins(const char* mixin, const char* attr, const sajson::value& in) {
        auto str = sajson::string("mesh", HA_COUNT_OF("mesh") - 1);
        if(in.find_object_key(str) != in.get_length()) {
            auto value = in.get_value_of_key(str);
            hassert(value.get_length() == 1);
            auto num_deserialized = deserialize(*this, value);
            hassert(num_deserialized == 1);
        }
    }
    void imgui_bind_attributes_mixins() {
        if(ImGui::TreeNode("mesh")) {
            auto changed_attr = imgui_bind_attributes(ha_this, "mesh", *this);
            if(changed_attr && attr_changed_callbacks.count(changed_attr)) {
                for(auto& cb : attr_changed_callbacks[changed_attr])
                    cb();
            }
            ImGui::TreePop();
        }
    }

    //HA_MESSAGES_IN_MIXIN(mesh)
public:
    mesh() {
        _path = "meshes/bunny.bin";

        _mesh   = MeshMan::get().get(_path);
        _shader = ShaderMan::get().get("mesh");

        attr_changed_callbacks["_path"].push_back([&]() {
            _mesh = MeshMan::get().get(_path);
            printf("yuhoo!\n");
        });
    }

    void get_rendering_parts(std::vector<renderPart>& out) const {
        out.push_back({_mesh, {}, _shader, tr::get_model_transform(ha_this)});
    }

    AABB get_aabb() const { return getMeshBBox(_mesh.get()); }
};

HA_MIXIN_DEFINE_WITHOUT_CODEGEN(
        mesh, common::serialize_mixins_msg& common::deserialize_mixins_msg&
                      common::set_attribute_mixins_msg& common::imgui_bind_attributes_mixins_msg&
                              rend::get_rendering_parts_msg& rend::get_aabb_msg);
//HA_MIXIN_DEFINE(mesh, rend::get_rendering_parts_msg& rend::get_aabb_msg);

class hierarchical
{
    HA_MESSAGES_IN_MIXIN(hierarchical)
    HA_FRIENDS_OF_TYPE(hierarchical);
    FIELD oid parent;
    FIELD std::vector<oid> children;

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

class selected
{
    HA_MESSAGES_IN_MIXIN(selected)
    HA_FRIENDS_OF_TYPE(selected);
    FIELD tinygizmo::rigid_transform gizmo_transform;
    FIELD tinygizmo::rigid_transform gizmo_transform_last;
    FIELD bool                       clicky = false;
    FIELD float                      dragy  = 42;
    FIELD double                     dragy2 = 42;
    FIELD int                        dragy3 = 42;
    FIELD std::string texty                 = "happy!!";
    FIELD std::string texty2                = ":(";

public:
    tinygizmo::rigid_transform& get_gizmo_transform() {
        gizmo_transform = tinygizmo::rigid_transform((const minalg::float4&)tr::get_rot(ha_this),
                                                     (const minalg::float3&)tr::get_pos(ha_this),
                                                     (const minalg::float3&)tr::get_scl(ha_this));
        return gizmo_transform;
    }

    tinygizmo::rigid_transform& get_last_stable_gizmo_transform() { return gizmo_transform_last; }

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

#include <gen/common.cpp.inl>
