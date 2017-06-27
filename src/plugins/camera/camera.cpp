#include "camera_gen.h"

#include "core/InputEvent.h"
#include "core/Application.h"

#include "core/messages/messages.h"
#include "core/messages/messages_camera.h"

using namespace dynamix;

const float k_speed = 20.f;

class HA_EMPTY_BASE camera : public camera_gen,
                             public InputEventListener<camera>,
                             public UpdatableMixin<camera>
{
    HA_MESSAGES_IN_MIXIN(camera)
public:
    camera() { set_pos(ha_this, glm::vec3(0, 50, 0)); }

    void process_event(const InputEvent& ev) {
        if(ev.type == InputEvent::MOTION) {
            cursor_x = float(ev.motion.x);
            cursor_y = float(ev.motion.y);
        }
    }

    void update(float dt) {
        uint32    w   = Application::get().width();
        uint32    h   = Application::get().height();
        glm::vec3 pos = get_pos(ha_this);
        if(cursor_x < 10)
            move(ha_this, glm::vec3(-k_speed * dt, 0, 0));
        if(cursor_x > w - 10)
            move(ha_this, glm::vec3(k_speed * dt, 0, 0));
        if(cursor_y < 10)
            move(ha_this, glm::vec3(0, 0, k_speed * dt));
        if(cursor_y > h - 10)
            move(ha_this, glm::vec3(0, 0, -k_speed * dt));
    }

    glm::mat4 get_view_matrix() {
        glm::vec3 pos = get_pos(ha_this);

        return glm::lookAtLH(glm::vec3(pos.x, pos.y, pos.z - 0.3f),
                             glm::vec3(pos.x, pos.y - 1, pos.z), glm::vec3(0, 0, 1));
    }

    glm::mat4 get_projection_matrix() {
        uint32 w = Application::get().width();
        uint32 h = Application::get().height();
        return glm::perspectiveLH(1.f, float(w) / float(h), 0.1f, 100.0f);
    }
};

HA_MIXIN_DEFINE(camera, process_event_msg& get_view_matrix_msg& get_projection_matrix_msg);
