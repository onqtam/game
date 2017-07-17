#include "camera_gen.h"

#include "core/InputEvent.h"
#include "core/Application.h"

#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>

#include "core/messages/messages.h"
#include "core/messages/messages_camera.h"

const float k_speed = 25.f;

const glm::vec3 k_init_look_direction = {0, -1, -0.2};
const glm::vec3 k_forward             = {0, 0, -1};
const glm::vec3 k_up                  = {0, 1, 0};
const glm::vec3 k_right               = {1, 0, 0};

class HA_EMPTY_BASE camera : public camera_gen,
                             public InputEventListener,
                             public UpdatableMixin<camera>
{
    HA_MESSAGES_IN_MIXIN(camera)
public:
    camera() {
        cursor_x = Application::get().width() / 2.f;
        cursor_y = Application::get().height() / 2.f;

        //set_pos(ha_this, glm::vec3(0, 0, 20));
        //set_rot(ha_this, glm::quat(1, 0, 0, 0));
        set_pos(ha_this, glm::vec3(0, 50, 2));
        set_rot(ha_this, Utils::rotationBetweenVectors(k_forward, k_init_look_direction));
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
        if(cursor_x < 10)
            move(ha_this, glm::vec3(-k_speed * dt, 0, 0));
        if(cursor_x > w - 10)
            move(ha_this, glm::vec3(k_speed * dt, 0, 0));
        if(cursor_y < 10)
            move(ha_this, glm::vec3(0, 0, -k_speed * dt));
        if(cursor_y > h - 10)
            move(ha_this, glm::vec3(0, 0, k_speed * dt));

        move(ha_this, glm::normalize(get_rot(ha_this) * k_forward) * scroll * 2.f);
        scroll = 0.f;
    }

    glm::mat4 get_view_matrix() {
        glm::mat4 t = glm::translate(glm::mat4(1.f), get_pos(ha_this));
        glm::mat4 r = glm::toMat4(get_rot(ha_this));
        return glm::inverse(t * r);
    }

    glm::mat4 get_projection_matrix() {
        uint32 w = Application::get().width();
        uint32 h = Application::get().height();
        return glm::perspective(glm::radians(45.0f), float(w) / float(h), 0.1f, 1000.0f);
    }
};

HA_MIXIN_DEFINE(camera, get_view_matrix_msg& get_projection_matrix_msg);
