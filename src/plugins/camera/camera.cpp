#include "core/serialization/serialization_common.h"
#include "core/imgui/imgui_bindings_common.h"

#include "core/InputEvent.h"
#include "core/Application.h"
#include "core/World.h"

#include "core/messages/messages_camera.h"

const float k_speed = 25.f;

class camera : public InputEventListener, public UpdatableMixin<camera>
{
    HA_MESSAGES_IN_MIXIN(camera);

    FIELD float cursor_x = 0.f;
    FIELD float cursor_y = 0.f;
    FIELD float scroll   = 0.f;

public:
    camera() {
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

        ha_this.move(move_vec);

        ha_this.move(yama::normalize(rotate(k_forward, ha_this.get_rot())) * scroll * 2.f);
        scroll = 0.f;
    }

    yama::matrix get_view_matrix() {
        auto t = yama::matrix::translation(ha_this.get_pos());
        auto r = yama::matrix::rotation_quaternion(ha_this.get_rot());
        return yama::inverse(t * r);
    }

    yama::matrix get_projection_matrix() {
        uint32 w = Application::get().width();
        uint32 h = Application::get().height();
        return yama::matrix::perspective_fov_rh(yama::deg_to_rad(45.0f), float(w) / float(h), 0.1f,
                                                1000.0f);
    }

    void no_gizmo() const {}
};

HA_MIXIN_DEFINE(camera, Interface_camera/*& no_gizmo_msg*/)

#include <gen/camera.cpp.inl>
