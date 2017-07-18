#pragma once

// clang-format off
#define HA_SINGLETON(the_class)                                                                    \
    public:                                                                                        \
        static the_class& get() { return *s_instance; }                                            \
    private:                                                                                       \
        static the_class* s_instance;                                                              \
        the_class(const the_class&) = delete;                                                      \
        the_class& operator=(const the_class&) = delete;                                           \
        friend struct Singleton<the_class>

#define HA_SINGLETON_INSTANCE(the_class) the_class* the_class::s_instance = nullptr
// clang-format on

template <class T>
struct Singleton
{
    Singleton(T* in) {
        hassert(T::s_instance == nullptr);
        T::s_instance = in;
    }
    ~Singleton() {
        hassert(T::s_instance != nullptr);
        T::s_instance = nullptr;
    }
};
