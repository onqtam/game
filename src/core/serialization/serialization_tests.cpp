#include "serialization.h"

template <typename T>
T getSomeVal();

test_case_template_define("[serialization]", T, serialization_template) {
    T data_in = getSomeVal<T>();

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

#define HA_TEMPLATE_VAL(type, ...)                                                                 \
    template <>                                                                                    \
    type getSomeVal<type>() {                                                                      \
        return __VA_ARGS__;                                                                        \
    }                                                                                              \
    test_case_template_instantiate(serialization_template, doctest::Types<type>)

HA_TEMPLATE_VAL(int, 42);
HA_TEMPLATE_VAL(float, 42.f);
HA_TEMPLATE_VAL(double, 42.);
HA_TEMPLATE_VAL(bool, false);
HA_TEMPLATE_VAL(std::vector<int>, {1, 2, 3});
HA_TEMPLATE_VAL(glm::vec3, {1, 2, 3});

#undef HA_TEMPLATE_VAL
