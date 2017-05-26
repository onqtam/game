#include "ObjectManager.h"

#include <iostream>

#include "mixins/messages/messages.h"
//#include "mixins/mixins/common_mixin_fwd.h"
//#include "mixins/mixins/exe_mixin_fwd.h"

#include "core/registry/registry.h"
#include "serialization/JsonData.h"

using namespace std;

HARDLY_SCOPED_SINGLETON_IMPLEMENT(ObjectManager);

void ObjectManager::init() {
    auto& mixins = getMixins();
    for(auto& mixin : mixins)
        cout << mixin.first << endl;

    addMixin(m_object, "common_mixin");
    //addMixin(m_object, "exe_mixin");

    set_id(m_object, 42);

    addMixin(m_object, "dummy");

    dynamix::object o1, o2, o3, o4;

    addMixin(o1, "dummy");
    addMixin(o2, "dummy");
    addMixin(o3, "dummy");
    remMixin(o1, "dummy");
    addMixin(o4, "dummy");

    addMixin(m_object, "trololo");

    JsonData state;
    state.reserve(1000);
    state.startObject();
    serialize(m_object, state);
    state.endObject();

    const sajson::document& doc  = state.parse();
    PPK_ASSERT(doc.is_valid());
    const sajson::value root = doc.get_root();
    deserialize(m_object, root);
}

void ObjectManager::update() {
    cout << " ====== trace ====== " << endl;
    trace(m_object, cout);

    auto& mixins = getMixins();
    for(auto& mixin : mixins)
        if(mixin.second.update)
            mixin.second.update();

    auto& globals = getGlobals();
    for(auto& global : globals)
        cout << global.first << endl;
}

void ObjectManager::addMixin(dynamix::object& obj, const char* mixin) {
    auto& mixins = getMixins();
    PPK_ASSERT(mixins.find(mixin) != mixins.end());
    mixins[mixin].add(&obj);
}

void ObjectManager::remMixin(dynamix::object& obj, const char* mixin) {
    auto& mixins = getMixins();
    PPK_ASSERT(mixins.find(mixin) != mixins.end());
    mixins[mixin].remove(&obj);
}
