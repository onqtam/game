#include "editor.h"

#include "core/serialization/serialization_common.h"

#include "core/messages/messages_editor.h"

HA_SUPPRESS_WARNINGS
#include <boost/range/adaptor/reversed.hpp>
HA_SUPPRESS_WARNINGS_END

HA_GCC_SUPPRESS_WARNING("-Wzero-as-null-pointer-constant") // because of boost::variant's ctor

static JsonData mixin_state(const Object& obj, cstr mixin) {
    JsonData out(1000);
    out.startObject();
    common::serialize_mixins(obj, mixin, out);
    out.endObject();
    return out;
}

static JsonData object_state(const Object& obj) {
    JsonData state(1000);
    state.startObject();
    state.append("\"\":");
    serialize(obj, state);
    state.endObject();
    return state;
}

static std::vector<std::string> mixin_names(const Object& obj) {
    std::vector<cstr> mixins;
    obj.get_mixin_names(mixins);
    std::vector<std::string> out(mixins.size());
    std::transform(mixins.begin(), mixins.end(), out.begin(), [](auto in) { return in; });
    return out;
}

static compound_cmd objects_set_parent(const std::vector<oid>& objects, oid new_parent) {
    compound_cmd comp_cmd;

    // save the transforms of the selected objects before changing parental information
    std::vector<std::pair<oid, std::pair<transform, JsonData>>> old_transforms;

    for(auto& curr : objects) {
        // record the old transform
        old_transforms.push_back(
                {curr, {tr::get_transform(curr.obj()), mixin_state(curr.obj(), "tform")}});

        // old parent old state
        auto     parent = get_parent(curr.obj());
        JsonData parent_old;
        if(parent)
            parent_old = mixin_state(parent.obj(), "parental");

        // record parental state of current object before change
        JsonData curr_old = mixin_state(curr.obj(), "parental");

        // set new parental relationship
        set_parent(curr.obj(), new_parent);

        // old parent new state & command submit
        if(parent)
            comp_cmd.commands.push_back(attributes_changed_cmd(
                    {parent, parent_old, mixin_state(parent.obj(), "parental")}));

        // current new state & command submit
        comp_cmd.commands.push_back(
                attributes_changed_cmd({curr, curr_old, mixin_state(curr.obj(), "parental")}));
    }

    for(auto& curr : old_transforms) {
        // set the old world transform (will recalculate the local transform of the object)
        tr::set_transform(curr.first.obj(), curr.second.first);
        // add the changed transform to the undo/redo command list
        comp_cmd.commands.push_back(attributes_changed_cmd(
                {curr.first, curr.second.second, mixin_state(curr.first.obj(), "tform")}));
    }

    return comp_cmd;
}

// TODO: remove this check once these are moved to the Object class
// also selected is problematic because we are looping over a container in it...
// also camera is problematic because it is registered as an input event listener and I got a crash when I undid it's removal...
static bool cant_remove_mixin(cstr in) {
    return strcmp(in, "tform") == 0 || strcmp(in, "parental") == 0 || strcmp(in, "selected") == 0 ||
           strcmp(in, "camera") == 0;
}

// =================================================================================================
// == EDITOR IMPLEMENTATION ========================================================================
// =================================================================================================

void editor::create_object() {
    printf("[CREATE OBJECT]\n");
    compound_cmd comp_cmd;

    auto& obj = ObjectManager::get().create();
    comp_cmd.commands.push_back(object_creation_cmd({obj.id(), object_state(obj), true}));
    comp_cmd.commands.push_back(
            object_mutation_cmd({obj.id(), mixin_names(obj), mixin_state(obj, nullptr), true}));

    comp_cmd.commands.push_back(update_selection_cmd({obj.id()}, m_selected));

    add_command(comp_cmd);
}

void editor::add_mixins_to_selected(std::vector<const mixin_type_info*> mixins) {
    compound_cmd comp_cmd;

    for(auto& id : m_selected) {
        auto& obj = id.obj();

        for(auto& mixin : mixins) {
            if(obj.has(mixin->id))
                continue;

            obj.addMixin(mixin->name);
            comp_cmd.commands.push_back(
                    object_mutation_cmd({id, {mixin->name}, mixin_state(obj, mixin->name), true}));
        }
    }

    if(!comp_cmd.commands.empty()) {
        printf("[ADD MIXINS]\n");
        add_command(comp_cmd);
    }
}

void editor::remove_mixins_by_name_from_selected(std::vector<const mixin_type_info*> mixins) {
    compound_cmd comp_cmd;

    for(auto& id : m_selected) {
        auto& obj = id.obj();

        for(auto& mixin : mixins) {
            if(!obj.has(mixin->id) || cant_remove_mixin(mixin->name))
                continue;

            obj.remMixin(mixin->name);
            comp_cmd.commands.push_back(
                    object_mutation_cmd({id, {mixin->name}, mixin_state(obj, mixin->name), false}));
        }
    }

    if(!comp_cmd.commands.empty()) {
        printf("[REMOVE MIXINS BY NAME]\n");
        add_command(comp_cmd);
    }
}

void editor::remove_selected_mixins() {
    compound_cmd comp_cmd;

    for(auto& id : m_selected) {
        auto&                       selected_mixins = (*id.obj().get<selected>()).selected_mixins;
        std::set<dynamix::mixin_id> mixins_to_delete;

        for(auto& mixin : selected_mixins) {
            if(cant_remove_mixin(mixin.second.c_str()))
                continue;

            // remove the mixin
            comp_cmd.commands.push_back(object_mutation_cmd(
                    {id, {mixin.second}, mixin_state(id.obj(), mixin.second.c_str()), false}));
            id.obj().remMixin(mixin.second.c_str());

            mixins_to_delete.insert(mixin.first);
        }

        // update the selected mixins list in "selected"
        if(!mixins_to_delete.empty()) {
            JsonData ov = mixin_attr_state("selected", "selected_mixins", selected_mixins);
            for(auto& curr : mixins_to_delete)
                selected_mixins.erase(curr);
            JsonData nv = mixin_attr_state("selected", "selected_mixins", selected_mixins);
            comp_cmd.commands.push_back(attributes_changed_cmd({id, ov, nv}));
        }
    }

    if(!comp_cmd.commands.empty()) {
        printf("[REMOVE SELECTED MIXINS]\n");
        add_command(comp_cmd);
    }
}

compound_cmd editor::update_selection_cmd(const std::vector<oid>& to_select,
                                          const std::vector<oid>& to_deselect) {
    compound_cmd comp_cmd;

    if(to_select.size() + to_deselect.size() > 0) {
        comp_cmd.commands.reserve(to_select.size() + to_deselect.size());

        auto add_mutate_command = [&](oid id, bool select) {
            comp_cmd.commands.push_back(object_mutation_cmd(
                    {id, {"selected"}, mixin_state(id.obj(), "selected"), select}));
        };

        for(auto curr : to_select) {
            add_mutate_command(curr, true);
            curr.obj().addMixin("selected");
        }
        for(auto curr : to_deselect) {
            add_mutate_command(curr, false);
            curr.obj().remMixin("selected");
        }
    }

    //re-update the list for later usage
    update_selected();

    return comp_cmd;
}

void editor::update_selection(const std::vector<oid>& to_select,
                              const std::vector<oid>& to_deselect) {
    auto command = update_selection_cmd(to_select, to_deselect);
    if(!command.commands.empty()) {
        printf("[SELECTION]\n");

        add_command(command);
    }
}

void editor::handle_gizmo_changes() {
    compound_cmd comp_cmd;

    for(auto& id : selected_with_gizmo) {
        auto old_t = id.obj().get<selected>()->old_local_t;
        auto new_t = tr::get_transform_local(id.obj());
        if(old_t.pos != new_t.pos) {
            JsonData ov = mixin_attr_state("tform", "pos", old_t.pos);
            JsonData nv = mixin_attr_state("tform", "pos", new_t.pos);
            comp_cmd.commands.push_back(attributes_changed_cmd({id, ov, nv}));
        }
        if(old_t.scl != new_t.scl) {
            JsonData ov = mixin_attr_state("tform", "scl", old_t.scl);
            JsonData nv = mixin_attr_state("tform", "scl", new_t.scl);
            comp_cmd.commands.push_back(attributes_changed_cmd({id, ov, nv}));
        }
        if(old_t.rot != new_t.rot) {
            JsonData ov = mixin_attr_state("tform", "rot", old_t.rot);
            JsonData nv = mixin_attr_state("tform", "rot", new_t.rot);
            comp_cmd.commands.push_back(attributes_changed_cmd({id, ov, nv}));
        }
        // update this - even though we havent started using the gizmo - or else this might break when deleting the object
        id.obj().get<selected>()->old_t       = tr::get_transform(id.obj());
        id.obj().get<selected>()->old_local_t = tr::get_transform_local(id.obj());
    }
    if(!comp_cmd.commands.empty())
        add_command(comp_cmd);
}

void editor::reparent(oid new_parent_for_selected) {
    // detect cycles - the new parent shouldn't be a child (close or distant) of any of the selected objects
    if(new_parent_for_selected) {
        for(auto curr = new_parent_for_selected; curr; curr = get_parent(curr.obj())) {
            if(std::find(m_selected.begin(), m_selected.end(), curr) != m_selected.end()) {
                printf("[REPARENT] CYCLE DETECTED! cannot reparent\n");
                new_parent_for_selected = oid::invalid(); // set it to an invalid state
            }
        }
    }

    // if selected objects have been dragged with the middle mouse button onto an unselected object - make them its children
    if(new_parent_for_selected) {
        printf("[REPARENT]\n");
        compound_cmd comp_cmd;

        auto new_parent_old = mixin_state(new_parent_for_selected.obj(), "parental");

        objects_set_parent(m_selected, new_parent_for_selected);

        // update the parental part of the new parent
        comp_cmd.commands.push_back(
                attributes_changed_cmd({new_parent_for_selected, new_parent_old,
                                        mixin_state(new_parent_for_selected.obj(), "parental")}));

        // add the compound command
        add_command(comp_cmd);
    }
}

void editor::group_selected() {
    if(m_selected.empty())
        return;

    printf("[GROUP]\n");
    compound_cmd comp_cmd;

    auto find_lowest_common_ancestor = [&]() {
        // go upwards from each selected node and update the visited count for each node
        std::map<oid, int> visited_counts;
        for(auto curr : m_selected) {
            while(curr != oid::invalid()) {
                visited_counts[curr]++;
                curr = get_parent(curr.obj());
            }
        }

        // remove any node that has been visited less times than the number of selected objects
        Utils::erase_if(visited_counts,
                        [&](auto in) { return in.second < int(m_selected.size()); });

        // if there is a common ancestor - it will have the same visited count as the number of selected objects
        if(visited_counts.size() == 1 &&
           std::find(m_selected.begin(), m_selected.end(), visited_counts.begin()->first) ==
                   m_selected.end()) {
            // if only one object is left after the filtering (common ancestor to all) and is not part of the selection
            return visited_counts.begin()->first;
        } else if(visited_counts.size() > 1) {
            // if atleast 2 nodes have the same visited count - means that one of the selected nodes
            // is also a common ancestor (also to itself) - we need to find it and get its parent
            for(auto& curr : visited_counts)
                if(curr.first.obj().has<selected>())
                    return get_parent(curr.first.obj());
        }
        // all other cases
        return oid::invalid();
    };

    // create new group object
    auto& group = ObjectManager::get().create("group");

    // if there is a common ancestor - add the new group object as its child
    auto common_ancestor = find_lowest_common_ancestor();
    if(common_ancestor) {
        JsonData ancestor_old = mixin_state(common_ancestor.obj(), "parental");
        set_parent(group, common_ancestor);
        comp_cmd.commands.push_back(attributes_changed_cmd(
                {common_ancestor, ancestor_old, mixin_state(common_ancestor.obj(), "parental")}));
    }

    // average position for the new group object
    auto average_pos = yama::vector3::zero();
    for(auto& curr : m_selected)
        average_pos += tr::get_pos(curr.obj());
    average_pos /= float(m_selected.size());
    // set position of newly created group to be the average position of all selected objects
    tr::set_transform(group, {average_pos, {1, 1, 1}, {0, 0, 0, 1}});

    objects_set_parent(m_selected, group.id());

    // add the created group object
    comp_cmd.commands.push_back(object_creation_cmd({group.id(), object_state(group), true}));
    comp_cmd.commands.push_back(object_mutation_cmd(
            {group.id(), mixin_names(group), mixin_state(group, nullptr), true}));

    // select the group and deselect the previously selected
    comp_cmd.commands.push_back(update_selection_cmd({group.id()}, m_selected));

    // add the compound command
    add_command(comp_cmd);
}

void editor::ungroup_selected() {
    if(m_selected.empty())
        return;

    compound_cmd comp_cmd;

    for(auto& curr : m_selected) {
        auto parent = get_parent(curr.obj());
        // skip selected objects that have no parents - they are already ungrouped
        if(!parent)
            continue;

        // record data before unparenting
        auto t            = tr::get_transform(curr.obj());
        auto curr_t_old   = mixin_state(curr.obj(), "tform");
        auto curr_p_old   = mixin_state(curr.obj(), "parental");
        auto parent_p_old = mixin_state(parent.obj(), "parental");

        // unaprent
        set_parent(curr.obj(), oid::invalid());
        // set the old world transform - will update the local transform
        tr::set_transform(curr.obj(), t);

        // submit commands with data after unparenting
        comp_cmd.commands.push_back(
                attributes_changed_cmd({curr, curr_t_old, mixin_state(curr.obj(), "tform")}));
        comp_cmd.commands.push_back(
                attributes_changed_cmd({curr, curr_p_old, mixin_state(curr.obj(), "parental")}));
        comp_cmd.commands.push_back(attributes_changed_cmd(
                {parent, parent_p_old, mixin_state(parent.obj(), "parental")}));
    }

    // add the compound command
    if(comp_cmd.commands.size()) {
        printf("[UNGROUP]\n");
        add_command(comp_cmd);
    }
}

void editor::duplicate_selected() {
    if(m_selected.empty())
        return;

    printf("[DUPLICATE]\n");
    compound_cmd comp_cmd;

    // filter out selected objects which are children (immediate or not) of other selected objects
    const std::set<oid> selected_set{m_selected.begin(), m_selected.end()};
    std::vector<oid>    top_most_selected;
    for(auto& curr : m_selected) {
        // search for the current object (and its parents) in the full set of selected objects
        oid  parent = curr;
        bool found  = false;
        do {
            parent = get_parent(parent.obj());
            if(selected_set.count(parent))
                found = true;
        } while(parent && !found);
        // if no parent of the current selected object is also in the set of selected objects
        if(!found)
            top_most_selected.push_back(curr);
    }

    // deselect the previously selected objects
    comp_cmd.commands.push_back(update_selection_cmd({}, m_selected));

    std::function<oid(Object&)> copy_recursive = [&](Object& to_copy) {
        // create a new object and copy
        auto& copy = ObjectManager::get().create();
        copy.copy_from(to_copy);

        // TODO: UGLY HACK: re-add the parental mixin to clear the broken parental relationships after the copy
        copy.remMixin("parental");
        copy.addMixin("parental");

        // save clean aprental state
        JsonData copy_old = mixin_state(copy, "parental");

        // add commands for the creation of the new copy
        comp_cmd.commands.push_back(object_creation_cmd({copy.id(), object_state(copy), true}));
        comp_cmd.commands.push_back(object_mutation_cmd(
                {copy.id(), mixin_names(copy), mixin_state(copy, nullptr), true}));

        // copy children recursively
        for(auto& child_to_copy : get_children(to_copy)) {
            auto     child_copy     = copy_recursive(child_to_copy.obj());
            JsonData child_copy_old = mixin_state(child_copy.obj(), "parental");
            // link parentally
            set_parent(child_copy.obj(), copy.id());
            // submit a command for that linking
            comp_cmd.commands.push_back(attributes_changed_cmd(
                    {child_copy, child_copy_old, mixin_state(child_copy.obj(), "parental")}));
        }

        // update parental information for the current copy
        comp_cmd.commands.push_back(
                attributes_changed_cmd({copy.id(), copy_old, mixin_state(copy, "parental")}));

        return copy.id();
    };

    // copy the filtered top-most objects + all their children (and select the new top-most object copies)
    std::vector<oid> new_top_most_selected;
    for(auto& curr : top_most_selected) {
        auto new_top = copy_recursive(curr.obj());
        new_top_most_selected.push_back(new_top);

        // if the current top-most object has a parent - make the copy a child of that parent as well
        auto curr_parent = get_parent(curr.obj());
        if(curr_parent) {
            JsonData new_top_old     = mixin_state(new_top.obj(), "parental");
            JsonData curr_parent_old = mixin_state(curr_parent.obj(), "parental");
            // link parentally
            set_parent(new_top.obj(), curr_parent);
            // submit a command for that linking
            comp_cmd.commands.push_back(attributes_changed_cmd(
                    {new_top, new_top_old, mixin_state(new_top.obj(), "parental")}));
            comp_cmd.commands.push_back(attributes_changed_cmd(
                    {curr_parent, curr_parent_old, mixin_state(curr_parent.obj(), "parental")}));
        }
    }

    // select the new top-most objects
    comp_cmd.commands.push_back(update_selection_cmd(new_top_most_selected, {}));

    // add the compound command
    add_command(comp_cmd);
}

void editor::delete_selected() {
    if(m_selected.empty())
        return;

    printf("[DELETE]\n");
    handle_gizmo_changes();

    compound_cmd comp_cmd;
    for(auto& curr : m_selected) {
        auto detele_object = [&](Object& obj) {
            // serialize the state of the mixins
            comp_cmd.commands.push_back(object_mutation_cmd(
                    {obj.id(), mixin_names(obj), mixin_state(obj, nullptr), false}));

            // serialize the state of the object itself
            comp_cmd.commands.push_back(object_creation_cmd({obj.id(), object_state(obj), false}));

            ObjectManager::get().destroy(obj.id());
        };

        std::function<void(oid)> delete_unselected_children = [&](oid root) {
            auto& root_obj = root.obj();
            // recurse through children
            const auto& children = ::get_children(root_obj);
            for(const auto& c : children)
                delete_unselected_children(c);
            // delete only if not selected because we are iterating through the selected objects anyway
            if(!root_obj.has<selected>())
                detele_object(root_obj);
        };

        delete_unselected_children(curr);
        detele_object(curr.obj());
    }
    add_command(comp_cmd);

    m_selected.clear();
}

void editor::undo() {
    if(curr_undo_redo >= 0) {
        printf("[UNDO] current action in undo/redo stack: %d (a total of %d actions)\n",
               curr_undo_redo - 1, int(undo_redo_commands.size()));
        handle_command(undo_redo_commands[curr_undo_redo--], true);
    }
}

void editor::redo() {
    if(curr_undo_redo + 1 < int(undo_redo_commands.size())) {
        printf("[REDO] current action in undo/redo stack: %d (a total of %d actions)\n",
               curr_undo_redo + 1, int(undo_redo_commands.size()));
        handle_command(undo_redo_commands[++curr_undo_redo], false);
    }
}

void editor::handle_command(command_variant& command_var, bool undo) {
    if(command_var.type() == boost::typeindex::type_id<attributes_changed_cmd>()) {
        auto&       cmd = boost::get<attributes_changed_cmd>(command_var);
        auto&       val = undo ? cmd.old_val : cmd.new_val;
        const auto& doc = val.parse();
        hassert(doc.is_valid());
        const auto root = doc.get_root();
        hassert(root.get_length() == 1);
        if(strcmp(root.get_object_key(0).data(), "") == 0)
            deserialize(cmd.e.obj(), root.get_object_value(0)); // object attribute
        else
            common::deserialize_mixins(cmd.e.obj(), root); // mixin attribute
    } else if(command_var.type() == boost::typeindex::type_id<object_mutation_cmd>()) {
        auto& cmd = boost::get<object_mutation_cmd>(command_var);
        if((!cmd.added && undo) || (cmd.added && !undo)) {
            // add the mixins
            for(auto& mixin : cmd.mixins)
                cmd.id.obj().addMixin(mixin.c_str());
            // deserialize the mixins
            const auto& doc = cmd.state.parse();
            hassert(doc.is_valid());
            common::deserialize_mixins(cmd.id.obj(), doc.get_root());
        } else {
            // remove the mixins
            for(auto& mixin : cmd.mixins)
                cmd.id.obj().remMixin(mixin.c_str());
        }
    } else if(command_var.type() == boost::typeindex::type_id<object_creation_cmd>()) {
        auto& cmd = boost::get<object_creation_cmd>(command_var);
        if((cmd.created && undo) || (!cmd.created && !undo)) {
            ObjectManager::get().destroy(cmd.id);
        } else {
            ObjectManager::get().createFromId(cmd.id);
            const auto& doc = cmd.state.parse();
            hassert(doc.is_valid());
            deserialize(cmd.id.obj(), doc.get_root().get_object_value(0)); // object attributes
        }
    } else if(command_var.type() == boost::typeindex::type_id<compound_cmd>()) {
        auto& cmd = boost::get<compound_cmd>(command_var);
        if(undo)
            for(auto& curr : boost::adaptors::reverse(cmd.commands))
                handle_command(curr, undo);
        else
            for(auto& curr : cmd.commands)
                handle_command(curr, undo);
    }
}

void editor::add_command(const command_variant& command) {
    if(!undo_redo_commands.empty())
        undo_redo_commands.erase(undo_redo_commands.begin() + 1 + curr_undo_redo,
                                 undo_redo_commands.end());

    undo_redo_commands.push_back(command);

    ++curr_undo_redo;
    printf("num actions in undo/redo stack: %d\n", curr_undo_redo);
}

void editor::add_changed_attribute(oid e, const JsonData& old_val, const JsonData& new_val) {
    add_command(attributes_changed_cmd({e, old_val, new_val}));
}

HA_GCC_SUPPRESS_WARNING_END
