#include "core/serialization/serialization_common.h"
#include "core/imgui/imgui_bindings_common.h"

#include "core/Input.h"
#include "core/Application.h"
#include "core/World.h"

#include "core/messages/messages_camera.h"

const float k_speed = 25.f;

static auto proj() {
    uint32 w = Application::get().width();
    uint32 h = Application::get().height();
    return yama::matrix::perspective_fov_rh(yama::deg_to_rad(45.0f), float(w) / float(h), 0.1f,
                                            1000.0f);
}

class gameplay_camera : public InputEventListener, public UpdatableMixin<gameplay_camera>
{
    HA_MESSAGES_IN_MIXIN(gameplay_camera);

    FIELD float cursor_x = 0.f;
    FIELD float cursor_y = 0.f;
    FIELD float scroll   = 0.f;

public:
    gameplay_camera() {
        cursor_x = Application::get().width() / 2.f;
        cursor_y = Application::get().height() / 2.f;

        ha_this.set_pos(yama::vector3::coord(0, 50, 2));
        ha_this.set_rot(yama::quaternion::rotation_vectors(k_forward, k_init_look_direction));
    }

    void process_event(const InputEvent& ev) override {
        if(ev.type == InputEvent::MOUSE) {
            cursor_x = float(ev.mouse.x);
            cursor_y = float(ev.mouse.y);
        }
        if(ev.type == InputEvent::SCROLL) {
            scroll += float(ev.scroll.scroll);
        }
    }

    void update(float dt) {
        uint32 w = Application::get().width();
        uint32 h = Application::get().height();

        auto move_vec = yama::vector3::zero();

        if(cursor_x < 10)
            move_vec += yama::vector3::coord(-k_speed * dt, 0, 0);
        if(cursor_x > w - 10)
            move_vec += yama::vector3::coord(k_speed * dt, 0, 0);
        if(cursor_y < 10)
            move_vec += yama::vector3::coord(0, 0, -k_speed * dt);
        if(cursor_y > h - 10)
            move_vec += yama::vector3::coord(0, 0, k_speed * dt);

        auto pos = ha_this.get_pos();
        pos += move_vec;
        auto fix_vec = yama::vector3::zero();
        auto half_w  = World::get().width() / 2;
        auto half_h  = World::get().height() / 2;
        if(abs(pos.x) > half_w)
            fix_vec.x = Utils::sign(pos.x) * half_w - pos.x;
        if(abs(pos.z) > half_h)
            fix_vec.z = Utils::sign(pos.z) * half_h - pos.z;

        move_vec += fix_vec;

        ha_this.move_local(move_vec);

        ha_this.move_local(yama::normalize(rotate(k_forward, ha_this.get_rot())) * scroll * 2.f);
        scroll = 0.f;
    }

    yama::matrix get_view_matrix() const {
        auto t = yama::matrix::translation(ha_this.get_pos());
        auto r = yama::matrix::rotation_quaternion(ha_this.get_rot());
        return yama::inverse(t * r);
    }

    yama::matrix get_projection_matrix() const { return proj(); }

    void no_gizmo() const {}
};

HA_MIXIN_DEFINE(gameplay_camera, Interface_camera /*& no_gizmo_msg*/)

class maya_camera : public InputEventListener, public UpdatableMixin<maya_camera>
{
    HA_MESSAGES_IN_MIXIN(maya_camera);

    FIELD float radius             = 50.f;
    FIELD float yaw                = yama::deg_to_rad(-90.f);
    FIELD float pitch              = yama::deg_to_rad(60.f);
    FIELD yama::vector3 pivotPoint = yama::vector3::uniform(0.f);

public:
    maya_camera() {
        ha_this.set_pos(eyePosition());
        ha_this.set_rot(yama::quaternion::rotation_x(pitch) * yama::quaternion::rotation_y(yaw));
    }

    void orientation(yama::vector3& pivotToEye, yama::vector3& up, yama::vector3& right) const {
        pivotToEye =
                yama::vector3::coord(cos(yaw) * cos(pitch), sin(pitch), -sin(yaw) * cos(pitch)) *
                radius;

        //pivotToEye = yama::vector3::coord(pivotToEye.z, pivotToEye.y, -pivotToEye.x);
        //pivotToEye = yama::vector3::coord(-pivotToEye.z, pivotToEye.y, pivotToEye.x);

        up = yama::normalize(
                yama::vector3::coord(cos(yaw) * cos(pitch + yama::constants::PI_HALF()),
                                     sin(pitch + yama::constants::PI_HALF()),
                                     -sin(yaw) * cos(pitch + yama::constants::PI_HALF())));

        right = yama::normalize(cross(up, pivotToEye));
    }

    yama::vector3 eyePosition() const {
        return pivotPoint +
               yama::vector3::coord(cos(yaw) * cos(pitch), sin(pitch), -sin(yaw) * cos(pitch)) *
                       radius;
    }

    yama::matrix get_view_matrix() const {
        //auto t = yama::matrix::translation(ha_this.get_pos());
        //auto r = yama::matrix::rotation_quaternion(ha_this.get_rot());
        //return yama::inverse(t * r);

        yama::vector3 pivotToEye, up, right;
        orientation(pivotToEye, up, right);
        auto cameraPosition = pivotPoint + pivotToEye;
        return yama::matrix::look_at_rh(cameraPosition, pivotPoint, up);
    }

    yama::matrix get_projection_matrix() const { return proj(); }

    void strafe(const float amountRight, const float amountUp) {
        yama::vector3 pivotToEye, up, right;
        orientation(pivotToEye, up, right);

        pivotPoint += amountRight * right;
        pivotPoint += amountUp * up;
    }

    void process_event(const InputEvent& ev) override {
        if(ev.type == InputEvent::MOUSE) {
            if(isModOn(HA_MOD_ALT)) {
                float dx = float(ev.mouse.dx);
                float dy = float(ev.mouse.dy);
                if(isButtonDown(MouseButton::Left) &&
                   !(isButtonDown(MouseButton::Right) || isButtonDown(MouseButton::Middle))) {
                    yaw += -dx * 0.01f;
                    pitch += dy * 0.01f;
                } else if(isButtonDown(MouseButton::Right)) {
                    radius += dy * 0.08f * radius * 0.1f;
                    if(radius < 0.1f)
                        radius = 0.1f;
                } else if(isButtonDown(MouseButton::Middle)) {
                    strafe(-dx * 0.007f * radius * 0.1f, dy * 0.007f * radius * 0.1f);
                }

                // always update the position and orientation of the camera object itself
                ha_this.set_pos(eyePosition());
                ha_this.set_rot(yama::quaternion::rotation_x(pitch) * yama::quaternion::rotation_y(yaw));
            }
        }
    }

    void update(float) {}
};

HA_MIXIN_DEFINE(maya_camera, Interface_camera /*& no_gizmo_msg*/)

#include <gen/camera.cpp.inl>
