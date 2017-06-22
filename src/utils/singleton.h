#pragma once

#include "visibility.h"

// clang-format off
#define HARDLY_SCOPED_SINGLETON(the_class, the_friend)                                             \
    private:                                                                                       \
        static HAPI the_class* s_instance;                                                         \
        the_class() {                                                                              \
            PPK_ASSERT(s_instance == nullptr);                                                     \
            s_instance = this;                                                                     \
        }                                                                                          \
        ~the_class() { s_instance = nullptr; }                                                     \
    public:                                                                                        \
        static the_class& get() { return *s_instance; }                                            \
    private:                                                                                       \
    friend the_friend

#define HARDLY_SCOPED_SINGLETON_IMPLEMENT(the_class) the_class* the_class::s_instance = nullptr
// clang-format on
