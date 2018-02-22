#include "core/serialization/serialization_common.h"
#include "core/imgui/imgui_bindings_common.h"
#include "core/Application.h"

HA_SINGLETON_INSTANCE(ObjectManager);

void Object::orphan() {
    if(m_parent) {
        auto& parent_ch         = m_parent.obj().m_children;
        auto  me_in_parent_iter = std::find(parent_ch.begin(), parent_ch.end(), m_id);
        hassert(me_in_parent_iter != parent_ch.end());
        std::swap(*me_in_parent_iter, parent_ch.back());
        parent_ch.pop_back();
    }
}

void Object::unparent() {
    while(m_children.size()) {
        auto& ch = m_children.back();
        hassert(ch);
        ch.obj().m_parent = oid::invalid();
        m_children.pop_back();
    }
}

Object::~Object() {
    if(
#ifndef DOCTEST_CONFIG_DISABLE
            id().isValid() &&
#endif
            Application::get().state() == Application::State::PLAY) {
        orphan();
        unparent();
    }
}

void Object::addMixin(cstr mixin) {
    auto& mixins = getAllMixins();
    hassert(mixins.find(mixin) != mixins.end());
    mixins[mixin].add(this);
}

void Object::remMixin(cstr mixin) {
    auto& mixins = getAllMixins();
    hassert(mixins.find(mixin) != mixins.end());
    mixins[mixin].remove(this);
}

void Object::set_transform(const transform& in) {
    if(m_parent) {
        auto child_local = in.multiply(m_parent.obj().get_transform().inverse());
        set_transform_local(child_local);
    } else {
        set_transform_local(in);
    }
}

transform Object::get_transform() const {
    transform my = get_transform_local();
    if(m_parent)
        return my.multiply(m_parent.obj().get_transform());
    else
        return my;
}

void Object::set_parent(oid parent) {
    orphan();
    m_parent = parent;
    if(m_parent != oid::invalid()) {
        hassert(m_parent);
        auto& parent_ch         = m_parent.obj().m_children;
        auto  me_in_parent_iter = std::find(parent_ch.begin(), parent_ch.end(), m_id);
        hassert(me_in_parent_iter == parent_ch.end());
        parent_ch.push_back(m_id);
    }
}

const Object& Object::dummy() {
    static Object out;
    return out;
}

// included here and not in Object.h to contain dependencies
#include <gen/Object.h.inl>

ObjectManager::~ObjectManager() {
    while(m_objects.size())
        m_objects.erase(m_objects.begin());
}
