#include "serialization_common.h"
#include "serialization_2.h"

bool operator==(const JsonData& lhs, const JsonData& rhs) { return lhs.data() == rhs.data(); }

namespace tinygizmo
{
bool operator==(const rigid_transform& lhs, const rigid_transform& rhs) {
    return lhs.position == rhs.position && lhs.scale == rhs.scale &&
           lhs.orientation == rhs.orientation;
}
bool operator==(const gizmo_application_state& lhs, const gizmo_application_state& rhs) {
    return memcmp(&lhs, &rhs, sizeof(lhs)) == 0;
}
} // namespace tinygizmo

template <typename T>
T getSomeVal();

template <typename T>
T& addApprox(T& in) {
    return in;
}
doctest::Approx addApprox(float in) { return doctest::Approx(in); }
doctest::Approx addApprox(double in) { return doctest::Approx(in); }

test_case_template_define("[serialization]", T, serialization_template) {
    const T data_in = getSomeVal<T>();

    JsonData state;

    state.startObject();
    state.append("\"data\":");
    serialize(data_in, state);
    state.endObject();

    const sajson::document& doc = state.parse();
    require_un(doc.is_valid());

    T                   data_out;
    const sajson::value root = doc.get_root();
    deserialize(data_out, root.get_object_value(0));
    check_eq(addApprox(data_in), data_out);
}

// helpers for the counting of serialization routine tests
const int serialize_tests_counter_start = __COUNTER__;

#define HA_SERIALIZE_TEST(type, ...)                                                               \
    template <>                                                                                    \
    type getSomeVal<type>() {                                                                      \
        return __VA_ARGS__;                                                                        \
    }                                                                                              \
    test_case_template_instantiate(serialization_template, doctest::Types<type>)

HA_SERIALIZE_TEST(char, 'g');
HA_SERIALIZE_TEST(int, 42);
HA_SERIALIZE_TEST(float, 42.f);
HA_SERIALIZE_TEST(double, 42.);
HA_SERIALIZE_TEST(bool, false);
HA_SERIALIZE_TEST(std::string, "hello !");
HA_SERIALIZE_TEST(yama::vector3, {1, 2, 3});
HA_SERIALIZE_TEST(transform, {{0, 1, 2}, {3, 4, 5}, {6, 7, 8, 9}});
HA_SERIALIZE_TEST(oid, oid(1));
HA_SERIALIZE_TEST(MeshHandle, MeshHandle());
HA_SERIALIZE_TEST(ShaderHandle, ShaderHandle());
HA_SERIALIZE_TEST(JsonData, JsonData());
HA_SERIALIZE_TEST(std::vector<int>, {1, 2, 3});

HA_SUPPRESS_WARNINGS
typedef boost::variant<int, char, double, float> variant_no_commas_for_test;
HA_SERIALIZE_TEST(variant_no_commas_for_test, 42.5);
HA_SUPPRESS_WARNINGS_END

// serialization_2.h
HA_SERIALIZE_TEST(tinygizmo::rigid_transform, {{0, 1, 2, 3}, {4, 5, 6}, {7, 8, 9}});
HA_SERIALIZE_TEST(tinygizmo::gizmo_application_state, tinygizmo::gizmo_application_state());

#undef HA_SERIALIZE_TEST

// helpers for the counting of serialization routine tests
const int num_serialize_tests = (__COUNTER__ - serialize_tests_counter_start - 1) / 2;
#ifdef _MSC_VER
// currently counts properly only on msvc... num_serialize_definitions is 0 on gcc
static_assert(num_serialize_tests == num_serialize_definitions + num_serialize_definitions_2,
              "forgot a serialization test?");
#endif // _MSC_VER
