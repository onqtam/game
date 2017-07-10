#include "ResourceManager.h"

int test_int_func_1(int a = 0, int b = 0) { return a + b; }
int test_int_func_2(int a = 42, int b = 666) { return a + b; }

struct TestIntCreator
{
    template <typename... Args>
    void create(void* storage, const std::string& name, Args&&... args) {
        if(name == "one")
            new(storage) int(test_int_func_1(std::forward<Args>(args)...));
        if(name == "two")
            new(storage) int(test_int_func_2(std::forward<Args>(args)...));
    }
    void destroy(void* storage) { *static_cast<int*>(storage) = -1; }
};

template class ResourceManager<int, TestIntCreator>;
typedef ResourceManager<int, TestIntCreator>         TestIntMan;
typedef ResourceManager<int, TestIntCreator>::Handle TestIntHandle;

test_case("[core] testing ResourceManager") {
    TestIntMan man;
    {
        // default constructed handles don't affect the manager
        TestIntHandle h0;
        check_eq(*reinterpret_cast<int16*>(&h0), -1);
        check_eq(man.numSlots(), 0);

        auto h1 = man.get("one");
        auto h2 = man.get("one");
        auto h3 = man.get("two");
        auto h4(h1);
        auto h5(std::move(h4)); // move from h4

        check_eq(h1.refcount(), 3);
        check_eq(h3.refcount(), 1);
        check_eq(*reinterpret_cast<int16*>(&h4), -1); // assert h4 has been moved from
        check_eq(h5.refcount(), 3);

        check_eq(h1.get(), 0);
        check_eq(h2.get(), 0);
        check_eq(h5.get(), 0);
        check_eq(h3.get(), 708);

        check_eq(man.numCanFree(), 0);
        check_eq(man.numFreeSlots(), 0);
        check_eq(man.numSlots(), 2);
    }
    check_eq(man.numCanFree(), 2);
    man.free();
    check_eq(man.numCanFree(), 0);
    check_eq(man.numFreeSlots(), 2);

    {
        // test forwarding constructors and hashing of arguments (for uniqueness of handles)
        auto h1 = man.get("one", 5, 6);
        auto h2 = man.get("one", 5, 7);
        check_eq(h1.get(), 11);
        check_eq(h2.get(), 12);
        check_eq(man.numCanFree(), 0);
        check_eq(man.numFreeSlots(), 0);
        check_eq(man.numSlots(), 2);

        // test assignment
        h2 = h1;
        check_eq(h2.get(), 11);
        check_eq(man.numCanFree(), 1);
        check_eq(man.numFreeSlots(), 0);
        check_eq(man.numSlots(), 2);

        // test reuse by taking into account the creation arguments (using their hash)
        auto h3 = man.get("one", 5, 7);
        check_eq(h3.get(), 12);
        check_eq(man.numCanFree(), 0);
        check_eq(man.numFreeSlots(), 0);
        check_eq(man.numSlots(), 2);
    }
}
