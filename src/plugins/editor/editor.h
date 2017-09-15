#pragma once

HA_SUPPRESS_WARNINGS
#include <tinygizmo/tiny-gizmo.hpp>
HA_SUPPRESS_WARNINGS_END

#include "core/registry/registry.h"
#include "core/GraphicsHelpers.h"
#include "core/InputEvent.h"

#include "core/messages/messages_editor.h"

//HA_GCC_SUPPRESS_WARNING("-Wzero-as-null-pointer-constant") // because of boost::variant's ctor

struct attributes_changed_cmd
{
    HA_FRIENDS_OF_TYPE(attributes_changed_cmd);

public:
    FIELD oid e;
    FIELD JsonData old_val;
    FIELD JsonData new_val;
};

struct object_creation_cmd
{
    HA_FRIENDS_OF_TYPE(object_creation_cmd);

public:
    FIELD oid id;
    FIELD JsonData state;
    FIELD bool     created;
};

struct object_mutation_cmd
{
    HA_FRIENDS_OF_TYPE(object_mutation_cmd);

public:
    FIELD oid id;
    FIELD std::vector<std::string> mixins;
    FIELD JsonData state;
    FIELD bool     added;
};

struct compound_cmd
{
    HA_FRIENDS_OF_TYPE(compound_cmd);

public:
    typedef boost::variant<attributes_changed_cmd, object_mutation_cmd, object_creation_cmd,
                           compound_cmd>
                                         command_variant;
    typedef std::vector<command_variant> commands_vector;

    FIELD commands_vector commands;
};

typedef compound_cmd::command_variant command_variant;
typedef compound_cmd::commands_vector commands_vector;

REFL_ATTRIBUTES(REFL_NO_INLINE)
class editor : public UpdatableMixin<editor>, public InputEventListener, public Singleton<editor>
{
    HA_SINGLETON(editor);
    HA_MESSAGES_IN_MIXIN(editor);

    FIELD std::vector<oid> selected;
    FIELD std::vector<oid> selected_with_gizmo;
    FIELD commands_vector undo_redo_commands;
    FIELD int             curr_undo_redo            = -1;
    FIELD bool            mouse_button_left_changed = false;
    FIELD tinygizmo::rigid_transform gizmo_transform;
    FIELD tinygizmo::rigid_transform gizmo_transform_last;

    // these members are OK to not be serialized because they are constantly updated
    tinygizmo::gizmo_application_state m_gizmo_state;
    tinygizmo::gizmo_context           m_gizmo_ctx;
    std::vector<char>                  m_verts;
    std::vector<char>                  m_inds;
    ShaderHandle                       m_program;
    bgfx_vertex_decl                   vd;
    bgfx_vertex_buffer_handle          m_vert_buf = {BGFX_INVALID_HANDLE};
    bgfx_index_buffer_handle           m_ind_buf  = {BGFX_INVALID_HANDLE};

    dynamix::mixin_id selected_mixin_id =
            dynamix::internal::domain::instance().get_mixin_id_by_name("selected");

    GeomHandle   m_grid;
    ShaderHandle m_grid_shader;

    void update_selected();
    void update_gui();
    void update_gizmo();

    void handle_gizmo_changes();

    void update_selection(const std::vector<oid>& to_select, const std::vector<oid>& to_deselect);
    void reparent(oid new_parent_for_selected);
    void group_selected();
    void ungroup_selected();
    void duplicate_selected();
    void delete_selected();

    void handle_command(command_variant& command, bool undo);
    void add_command(const command_variant& command);

public:
    editor();
    ~editor();

    void update(float);

    void process_event(const InputEvent& ev) override;

    void add_changed_attribute(oid e, const JsonData& old_val, const JsonData& new_val);
};

//HA_GCC_SUPPRESS_WARNING_END
