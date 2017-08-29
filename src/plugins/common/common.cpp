#include "core/registry/registry.h"
#include "core/serialization/serialization.h"
#include "core/serialization/serialization_2.h"
#include "core/imgui/imgui_stuff.h"

#include "core/GraphicsHelpers.h"

#include "core/messages/messages.h"
#include "core/messages/messages_rendering.h"

class tform
{
    HA_MESSAGES_IN_MIXIN(tform);
    FIELD transform t = {{0, 0, 0}, {1, 1, 1}, {1, 0, 0, 0}};

public:
	void set_pos(const glm::vec3& in) { t.pos = in; }
	void set_scl(const glm::vec3& in) { t.scl = in; }
	void set_rot(const glm::quat& in) { t.rot = in; }

	const glm::vec3& get_pos() const { return t.pos; }
	const glm::vec3& get_scl() const { return t.scl; }
	const glm::quat& get_rot() const { return t.rot; }

    void set_transform(const transform& in) { t = in; }
	const transform& get_transform() const { return t; }

    void move(const glm::vec3& in) { t.pos += in; }

    glm::mat4 get_transform_mat() const {
        glm::mat4 tr = glm::translate(glm::mat4(1.f), t.pos);
        glm::mat4 rt = glm::toMat4(t.rot);
        return glm::scale(tr * rt, t.scl);
    }
};

HA_MIXIN_DEFINE(tform, Interface_transform);

class mesh
{
    HA_FRIENDS_OF_TYPE(mesh);
    ATTRIBUTES(tag::mesh)
    FIELD std::string _path;
    ATTRIBUTES(tag::image)
    FIELD std::string _image_path;
    FIELD MeshHandle _mesh;
    FIELD ShaderHandle _shader;

    FIELD bool   clicky      = false;
    FIELD float  dragy       = 42;
    FIELD double dragy2      = 42;
    FIELD int    dragy3      = 42;
    FIELD std::string texty  = "happy!!";
    FIELD std::string texty2 = ":(";

public:
    std::map<std::string, std::vector<std::function<void(void)>>> attr_changed_callbacks;

    void serialize_mixins(const char* concrete_mixin, JsonData& out) const {
        if(concrete_mixin && strcmp("mesh", concrete_mixin) != 0)
            return;
        out.append("\"mesh\":");
        serialize(*this, out);
        out.addComma();
    }
    void deserialize_mixins(const sajson::value& in) {
        auto str = sajson::string("mesh", HA_COUNT_OF("mesh") - 1);
        if(in.find_object_key(str) != in.get_length())
            deserialize(*this, in.get_value_of_key(str));
    }
    void set_attribute_mixins(const char*, const char*, const sajson::value& in) {
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
        out.push_back({_mesh, {}, _shader, tr::get_transform_mat(ha_this)});
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
	HA_MESSAGES_IN_MIXIN(hierarchical);
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
    HA_MESSAGES_IN_MIXIN(selected);
    FIELD transform old_t;

public:
    void get_rendering_parts(std::vector<renderPart>& out) const {
        if(ha_this.implements(rend::get_aabb_msg)) {
            auto diag   = rend::get_aabb(ha_this).getDiagonal();
            auto geom   = GeomMan::get().get("", createBox, diag.x, diag.y, diag.z, colors::green);
            auto shader = ShaderMan::get().get("cubes");
            out.push_back({{}, geom, shader, tr::get_transform_mat(ha_this)});
        }
    }

    transform& get_transform_on_gizmo_start() { return old_t; }
};

HA_MIXIN_DEFINE(selected, sel::get_transform_on_gizmo_start_msg& rend::get_rendering_parts_msg);

#include <gen/common.cpp.inl>
