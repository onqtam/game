#pragma once

// clang-format off
#define HA_SCOPED_SINGLETON(the_class)                                                             \
    public:                                                                                        \
        static the_class& get() { return *SingletonInstanceWrapper<the_class>::s_instance; }       \
    private:                                                                                       \
        SingletonInstanceWrapper<the_class> _singleton_instance_dummy =                            \
                SingletonInstanceWrapper<the_class>(this);                                         \
        the_class(const the_class&) = delete;                                                      \
        the_class& operator=(const the_class&) = delete
// clang-format on

// this helper class is used so singletons can have user defined ctors/dtors - the global instance
// handling is separated inside of a dummy member of the singleton class of type this helper class 
template <typename T>
struct SingletonInstanceWrapper
{
    static T* s_instance;
    SingletonInstanceWrapper(T* in) {
        hassert(s_instance == nullptr);
        s_instance = in;
    }
    ~SingletonInstanceWrapper() { s_instance = nullptr; }
};

template <class T>
T* SingletonInstanceWrapper<T>::s_instance = nullptr;
