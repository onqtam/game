#include "PagedMixinAllocator.h"

#define test_alloc(x) a.alloc_mixin(0, 0, x, offset_for_value)
#define test_dealloc(x) a.dealloc_mixin(x)
#define test_write(x, val) *(reinterpret_cast<int*>(x + offset_for_value)) = val

test_case("[core] testing PagedMixinAllocator with int") {
    char*  res_0;
    char*  res_1;
    char*  res_2;
    char*  res_3;
    char*  res_4;
    char*  res_5;
    char*  res_6;
    char*  res_7;
    char*  res_8;
    char*  res_9;
    size_t offset_for_value;

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
