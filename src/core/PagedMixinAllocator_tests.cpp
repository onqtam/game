#include "PagedMixinAllocator.h"

struct test_struct
{
    int data;
};

HA_MIXIN_DEFINE_WITHOUT_CODEGEN(test_struct, dynamix::none);

#define test_alloc(x) x = a.alloc_mixin(mixin_info, &Object::dummy())
#define test_dealloc(x) a.dealloc_mixin(x.first, 0, mixin_info, &Object::dummy())
#define test_write(x, val) (reinterpret_cast<test_struct*>(x.first + x.second))->data = val

test_case("[core] testing PagedMixinAllocator with int") {
    auto& mixin_info = _dynamix_get_mixin_type_info((test_struct*)nullptr);

    std::pair<char*, size_t> res_0;
    std::pair<char*, size_t> res_1;
    std::pair<char*, size_t> res_2;
    std::pair<char*, size_t> res_3;
    std::pair<char*, size_t> res_4;
    std::pair<char*, size_t> res_5;
    std::pair<char*, size_t> res_6;
    std::pair<char*, size_t> res_7;
    std::pair<char*, size_t> res_8;
    std::pair<char*, size_t> res_9;

    PagedMixinAllocator<int, 3> a;

    check_eq(a.getNumAllocations(), 0u);

    // make some allocations and fill with some data
    test_alloc(res_0);
    test_alloc(res_1);
    test_alloc(res_2);
    test_alloc(res_3);
    test_alloc(res_4);
    test_alloc(res_5);
    test_alloc(res_6);
    test_write(res_0, 0);
    test_write(res_1, 1);
    test_write(res_2, 2);
    test_write(res_3, 3);
    test_write(res_4, 4);
    test_write(res_5, 5);
    test_write(res_6, 6);

    // make a hole
    test_dealloc(res_3);

    // test flags and values of the elements inside
    const auto& flags = a.getAllocatedFlags();
    for(size_t i = 0; i < flags.size(); ++i)
        if(flags[i])
            check_eq(a[i], int(i));
    check_eq(a.getNumAllocations(), 6u);

    // check that it behaves as a LIFO - reusing the last deallocation for the last allocation
    test_alloc(res_7);

    check_eq(res_3, res_7);
    check_eq(a.getNumAllocations(), 7u);

    test_alloc(res_3);
    test_alloc(res_8);
    test_alloc(res_9);

    check_eq(a.getNumAllocations(), 10u);

    test_dealloc(res_9);
    test_dealloc(res_7);
    test_dealloc(res_5);
    test_dealloc(res_3);
    test_dealloc(res_1);
    test_dealloc(res_0);
    test_dealloc(res_2);
    test_dealloc(res_4);
    test_dealloc(res_6);
    test_dealloc(res_8);

    // check if all is cleared
    check_eq(a.getNumAllocations(), 0u);
    for(size_t i = 0; i < flags.size(); ++i)
        check_not(flags[i]);
}

#undef test_alloc
#undef test_dealloc
#undef test_write
