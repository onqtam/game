#include "serialization.h"

template <typename T>
T getSomeVal();

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

    check_eq(data_in, data_out);
}

#define HA_SERIALIZE_TEST(type, ...)                                                               \
    template <>                                                                                    \
    HA_USE(__COUNTER__)                                                                            \
    type getSomeVal<type>() {                                                                      \
        return __VA_ARGS__;                                                                        \
    }                                                                                              \
    test_case_template_instantiate(serialization_template, doctest::Types<type>)

// helpers for the counting of serialization routine tests
const int serialize_tests_counter_start = __COUNTER__;

HA_SERIALIZE_TEST(int, 42);
HA_SERIALIZE_TEST(float, 42.f);
HA_SERIALIZE_TEST(double, 42.);
HA_SERIALIZE_TEST(bool, false);
HA_SERIALIZE_TEST(std::vector<int>, {1, 2, 3});
HA_SERIALIZE_TEST(glm::vec3, {1, 2, 3});
HA_SERIALIZE_TEST(eid, eid(1));

// helpers for the counting of serialization routine tests
const int num_serialize_tests = (__COUNTER__ - serialize_tests_counter_start - 1) / 2;
static_assert(num_serialize_tests == num_serialize_definitions, "forgot a serialization test?");

#undef HA_SERIALIZE_TEST
