class XYZ
{
public:
    float32 x;
    float32 y;
    float32 z;
    
    template <typename Visitor>
    void VisitFields(Visitor& visitor)
    {
        visitor("x", x);
        visitor("y", y);
        visitor("z", z);
    }
    
    template <typename Visitor>
    void VisitFields(Visitor& visitor) const
    {
        template <typename T, typename Visitor)>
        struct vv 
        {
            template <typename M>
            void operator()(const char* name, M T::*m)
            {
                v(name, self->*m);
            }
            T* self;
            Visitor& v;
        };
        visit_members(vv(this));
    }
    
    template <typename Visitor>
    static void VisitMembers(Visitor& visitor)
    {
        visitor("x", &XYZ::x);
        visitor("y", &XYZ::y);
        visitor("z", &XYZ::z);
    }
};











template <typename>
struct has_oid : std::false_type
{};

template <>
struct has_oid<oid> : std::true_type
{};

template <typename T>
struct has_oid<std::vector<T>> : has_oid<T>
{};

template <typename K, typename V>
struct has_oid<std::map<K, V>> : has_oid<V>
{};

void visit(mesh& in, visitor& v) {
    v(&in, typeid(in));
    if constexpr(has_oid<decltype(in.clicky)>::value)      { visit(in.clicky, v); }
    if constexpr(has_oid<decltype(in.dragy)>::value)       { visit(in.dragy, v); }
    if constexpr(has_oid<decltype(in.dragy3)>::value)      { visit(in.dragy3, v); }
    if constexpr(has_oid<decltype(in.some_obj_id)>::value) { visit(in.some_obj_id, v); }
    if constexpr(has_oid<decltype(in.omg)>::value)  { visit(in.omg, v); }
    if constexpr(has_oid<decltype(in.omg2)>::value) { visit(in.omg2, v); }
}








