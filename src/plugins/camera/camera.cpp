#include "camera_gen.h"

#include "core/InputEvent.h"
#include "core/Application.h"

#include "core/messages/messages.h"
#include "core/messages/messages_camera.h"

test_case("") {}

using namespace dynamix;

class camera : public camera_gen, public InputEventListener<camera>
{
    HARDLY_MESSAGES_IN_MIXIN(common)
public:
    camera() { set_pos(ha_this, glm::vec3(0, 50, 0)); }

    void process_event(const InputEvent& ev) {
        if(ev.type == InputEvent::MOTION) {
            glm::vec3 pos = get_pos(ha_this);
            pos.x += ev.motion.dx * 0.01;
            pos.z += ev.motion.dy * 0.01;
            set_pos(ha_this, pos);
        }
    }

    glm::mat4 get_view_matrix() {
        glm::vec3 pos = get_pos(ha_this);

        return glm::lookAtLH(glm::vec3(pos.x, pos.y, pos.z), glm::vec3(pos.x, pos.y - 1, pos.z),
                             glm::vec3(0, 0, 1));
    }

    glm::mat4 get_projection_matrix() {
        uint32 w = Application::get().getWidth();
        uint32 h = Application::get().getHeight();
        return glm::perspectiveLH(1.f, float(w) / float(h), 0.1f, 100.0f);
    }
};

HARDLY_MIXIN(camera, process_event_msg& get_view_matrix_msg& get_projection_matrix_msg);
