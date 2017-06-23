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
    void process_event(const InputEvent& ev) { printf("event!\n"); }

    glm::mat4 get_view_matrix() {
        set_pos(ha_this, glm::vec3(0, 50, 0));
        glm::vec3 pos = get_pos(ha_this);

        return glm::lookAtLH(glm::vec3(pos.x, pos.y, pos.z), glm::vec3(pos.x, pos.y - 1, pos.z),
                             glm::vec3(0, 0, 1));
    }

    glm::mat4 get_projection_matrix() {
        return glm::perspectiveLH(
                1.f, float(Application::get().getWidth()) / float(Application::get().getHeight()),
                0.1f, 100.0f);
    }
};

HARDLY_MIXIN(camera, process_event_msg& get_view_matrix_msg& get_projection_matrix_msg);
