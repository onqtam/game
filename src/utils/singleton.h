#pragma once

// can perhaps rework this to not use macros at all - https://stackoverflow.com/a/4173298/3162383

// clang-format off
#define HA_SCOPED_SINGLETON(the_class, the_friend)                                                 \
    private:                                                                                       \
        static the_class* s_instance;                                                              \
        the_class() {                                                                              \
            hassert(s_instance == nullptr);                                                        \
            s_instance = this;                                                                     \
        }                                                                                          \
        the_class(const the_class&) = delete;                                                      \
        the_class& operator=(const the_class&) = delete;                                           \
        ~the_class() { s_instance = nullptr; }                                                     \
    public:                                                                                        \
        static the_class& get() { return *s_instance; }                                            \
    private:                                                                                       \
    friend the_friend

#define HA_SCOPED_SINGLETON_IMPLEMENT(the_class) the_class* the_class::s_instance = nullptr
// clang-format on
