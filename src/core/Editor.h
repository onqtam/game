#pragma once

#include "InputEvent.h"

#include "tinygizmo/tiny-gizmo.hpp"

class HAPI Editor : public InputEventListener
{
    HA_SCOPED_SINGLETON(Editor, class Application);

private:
    tinygizmo::gizmo_application_state m_gizmo_state;
    tinygizmo::gizmo_context           m_gizmo_ctx;
    tinygizmo::rigid_transform         m_transform;

    std::vector<char>        m_verts;
    std::vector<char>        m_inds;
    bgfx::ProgramHandle      m_program;
    bgfx::VertexBufferHandle m_vert_buf = BGFX_INVALID_HANDLE;
    bgfx::IndexBufferHandle  m_ind_buf  = BGFX_INVALID_HANDLE;
    bgfx::VertexDecl         m_vert_decl;

public:
    void init();
    void update();

    void process_event(const InputEvent& ev) override;
};
