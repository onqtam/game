#include "camera_gen.h"

#include "core/registry/registry.h"
#include "core/InputEvent.h"
#include "core/Application.h"

#include "mixins/messages/messages.h"
#include "mixins/messages/messages_camera.h"

#include <bx/fpumath.h>
#include <bgfx/bgfx.h>

#include "utils/doctest/doctest_proxy.h"

#include <iostream>

test_case("") {
    check_eq(1, 2);
}

using namespace dynamix;

class camera : public camera_gen//, public InputEventListener<camera>
{
    HARDLY_MESSAGES_IN_MIXIN(common)
public:
    void process_event(const InputEvent& ev) { std::cout << "event!\n"; }

    glm::mat4 get_view_matrix() {
        set_pos(dm_this, glm::vec3(0, 30, 0));
        glm::vec3 pos = get_pos(dm_this);
        
        PPK_ASSERT(false);
        return glm::lookAtLH(glm::vec3(pos.x, pos.y, pos.z), glm::vec3(pos.x, pos.y - 1, pos.z), glm::vec3(0, 0, 1));
        

        //float at[3]  = {0.0f, 0.0f, 0.0f};
        //float eye[3] = {0.0f, 40.0f, 0.0f};
        //float up[3]  = {0.0f, 0.0f, 1.0f};
        //
        //float view[16];
        //bx::mtxLookAt(view, eye, at, up);

        //return glm::mat4(view[0], view[1], view[2], view[3],   //
        //                 view[4], view[5], view[6], view[7],   //
        //                 view[8], view[9], view[10], view[11], //
        //                 view[12], view[13], view[14], view[15]);
    }

    glm::mat4 get_projection_matrix() {
        float proj[16];
        //bx::mtxProj(proj, 60.0f,
        //            float(1280) / float(768),
        //            0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth);
        bx::mtxProj(proj, 60.0f,
                    float(Application::get().getWidth()) / float(Application::get().getHeight()),
                    0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth);
        return glm::mat4(proj[0], proj[1], proj[2], proj[3],   //
                         proj[4], proj[5], proj[6], proj[7],   //
                         proj[8], proj[9], proj[10], proj[11], //
                         proj[12], proj[13], proj[14], proj[15]);
    }
};

HARDLY_MIXIN(camera, process_event_msg& get_view_matrix_msg& get_projection_matrix_msg);
