#include "core/registry/registry.h"
#include "core/serialization/serialization.h"
#include "core/imgui/imgui_stuff.h"

#include "core/InputEvent.h"
#include "core/Application.h"
#include "core/World.h"

#include "core/messages/messages.h"
#include "core/messages/messages_camera.h"

const float k_speed = 25.f;

const yama::vector3 k_init_look_direction = yama::normalize(yama::v(0, -1, -0.2f));
const yama::vector3 k_forward             = {0, 0, -1};
//const yama::vector3 k_up                  = {0, 1, 0};
//const yama::vector3 k_right               = {1, 0, 0};

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

        tr::set_pos(ha_this, yama::vector3::coord(0, 50, 2));
        tr::set_rot(ha_this, yama::quaternion::rotation_vectors(k_forward, k_init_look_direction));
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

        auto pos = tr::get_pos(ha_this);
        pos += move_vec;
        auto      fix_vec = yama::vector3::zero();
        auto      half_w = World::get().width() / 2;
        auto      half_h = World::get().height() / 2;
        if(abs(pos.x) > half_w)
            fix_vec.x = Utils::sign(pos.x) * half_w - pos.x;
        if(abs(pos.z) > half_h)
            fix_vec.z = Utils::sign(pos.z) * half_h - pos.z;

        move_vec += fix_vec;

        tr::move(ha_this, move_vec);

        tr::move(ha_this, yama::normalize(rotate(k_forward, tr::get_rot(ha_this))) * scroll * 2.f);
        scroll = 0.f;
    }

    yama::matrix get_view_matrix() {
        auto t = yama::matrix::translation(tr::get_pos(ha_this));
        auto r = yama::matrix::rotation_quaternion(tr::get_rot(ha_this));
        return yama::inverse(t * r);
    }

    yama::matrix get_projection_matrix() {
        uint32 w = Application::get().width();
        uint32 h = Application::get().height();
        return yama::matrix::perspective_fov_rh(yama::deg_to_rad(45.0f), float(w) / float(h), 0.1f, 1000.0f);
    }

    void no_gizmo() const {}
};

HA_MIXIN_DEFINE(camera,
                cam::get_view_matrix_msg& cam::get_projection_matrix_msg& sel::no_gizmo_msg);

#include <gen/camera.cpp.inl>
