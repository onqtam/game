#include "common_gen.h"

#include "core/registry/registry.h"
#include "core/ObjectManager.h"

#include "core/ResourceManager.h"

#include "core/messages/messages.h"

#include <iostream>

using namespace dynamix;
//using namespace std;

static void f() {
    intHandle ih = intMan::get().get("");
    ih.get() = 6;
    intHandle ih2 = intMan::get().get("");
    if(ih.get() == ih2.get())
        printf("lala");
}

class common : public common_gen
{
    HA_MESSAGES_IN_MIXIN(common)
public:
    void trace(std::ostream& o) const { o << " object with id " << int(ha_this.id()) << std::endl; }

    void set_pos(const glm::vec3& in) { pos = in; }
    void move(const glm::vec3& in) { pos += in; }
    const glm::vec3&           get_pos() const { return pos; }

    //void select(bool _in) { selected = _in; }
};

HA_MIXIN_DEFINE(common, get_pos_msg& set_pos_msg& move_msg& priority(1000, trace_msg));

class hierarchical : public hierarchical_gen
{
    HA_MESSAGES_IN_MIXIN(hierarchical)
public:
    eid                     get_parent() const { return parent; }
    const std::vector<eid>& get_children() const { return children; }

    void set_parent(eid _parent) {
        
        f();

        intMan::get().free();

        PPK_ASSERT(parent == eid::invalid());
        parent = _parent;
        //::add_child(ObjectManager::get().getObject(_parent), ha_this.id());
    }
    void add_child(eid child) {
        //PPK_ASSERT(::get_parent(ObjectManager::get().getObject(child)) == eid::invalid());
        PPK_ASSERT(std::find(children.begin(), children.end(), child) == children.end());
        children.push_back(child);
        //::set_parent(ObjectManager::get().getObject(child), ha_this.id());
    }
    void remove_child(eid child) {
        auto it = std::find(children.begin(), children.end(), child);
        PPK_ASSERT(it != children.end());
        children.erase(it);
    }
};

HA_MIXIN_DEFINE(hierarchical,
                get_parent_msg& get_children_msg& set_parent_msg& add_child_msg& remove_child_msg);
