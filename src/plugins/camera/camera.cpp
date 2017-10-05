#include "core/serialization/serialization_common.h"
#include "core/imgui/imgui_bindings_common.h"

#include "core/InputEvent.h"
#include "core/Application.h"
#include "core/World.h"

#include "core/messages/messages_camera.h"

HA_SUPPRESS_WARNINGS
#include <GLFW/glfw3.h>
HA_SUPPRESS_WARNINGS_END
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
    }

    void process_event(const InputEvent& ev) override {
        if(ev.type == InputEvent::MOTION) {
            cursor_x = float(ev.motion.x);
            cursor_y = float(ev.motion.y);
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

    FIELD bool  alt        = false;
    FIELD int   buttons[3] = {0, 0, 0};
    FIELD float cursor_x   = 0.f;
    FIELD float cursor_y   = 0.f;
    FIELD float scroll     = 0.f;

public:
    maya_camera() {
        cursor_x = Application::get().width() / 2.f;
        cursor_y = Application::get().height() / 2.f;
    }

    FIELD float radius               = 2.f;
    FIELD float yaw                  = 0.f;
    FIELD float pitch                = 0.f;
    FIELD yama::vector3 orbitPoint   = yama::vector3::uniform(0.f);
    FIELD yama::vector2 prevMousePos = yama::vector2::uniform(0.f);

    void orientation(yama::vector3& orbitToEye, yama::vector3& up, yama::vector3& right) const {
        orbitToEye =
                yama::vector3::coord(cos(yaw) * cos(pitch), sin(pitch), -sin(yaw) * cos(pitch)) *
                radius;

        up = yama::normalize(
                yama::vector3::coord(cos(yaw) * cos(pitch + yama::constants::PI_HALF()),
                                     sin(pitch + yama::constants::PI_HALF()),
                                     -sin(yaw) * cos(pitch + yama::constants::PI_HALF())));

        right = yama::normalize(cross(up, orbitToEye));
    }

    yama::vector3 eyePosition() const {
        return orbitPoint +
               yama::vector3::coord(cos(yaw) * cos(pitch), sin(pitch), -sin(yaw) * cos(pitch)) *
                       radius;
    }

    yama::matrix get_view_matrix() const {
        yama::vector3 orbitToEye, up, right;
        orientation(orbitToEye, up, right);

        auto cameraPosition = orbitPoint + orbitToEye;
        return yama::matrix::look_at_rh(cameraPosition, orbitPoint, up);
    }

    yama::matrix get_projection_matrix() const { return proj(); }

    void strafe(const float amountRight, const float amountUp) {
        yama::vector3 orbitToEye, up, right;
        orientation(orbitToEye, up, right);

        orbitPoint += amountRight * right;
        orbitPoint += amountUp * up;
    }

    void process_event(const InputEvent& ev) override {
        if(ev.type == InputEvent::MOTION) {
            if(alt) {
                float dx = float(ev.motion.dx);
                float dy = float(ev.motion.dy);
                if(buttons[0] && !(buttons[1] || buttons[2])) {
                    yaw += -dx * 0.01f;
                    pitch += dy * 0.01f;
                } else if(buttons[1]) {
                    radius += dy * 0.08f * radius * 0.1f;
                    if(radius < 0.1f)
                        radius = 0.1f;
                } else if(buttons[2]) {
                    strafe(-dx * 0.007f * radius * 0.1f, dy * 0.007f * radius * 0.1f);
                }
            }
        }
        if(ev.type == InputEvent::BUTTON) {
            buttons[ev.button.button] = ev.button.action;
        }
        if(ev.type == InputEvent::KEY) {
            alt = ev.key.mods & GLFW_MOD_ALT;
        }
    }

    void update(float) {}
};

HA_MIXIN_DEFINE(maya_camera, Interface_camera /*& no_gizmo_msg*/)

#include <gen/camera.cpp.inl>
