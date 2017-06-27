#ifndef DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
#define DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
#endif // DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES

#ifndef DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#endif // DOCTEST_CONFIG_SUPER_FAST_ASSERTS

// no guard for including the doctest header itself because it should support multiple inclusion
#include <doctest.h>

#ifndef MY_PROXY_MACROS
#define MY_PROXY_MACROS

// clang-format off

#define test_case                       DOCTEST_TEST_CASE
#define test_case_fixture               DOCTEST_TEST_CASE_FIXTURE
//#define type_to_string                  DOCTEST_TYPE_TO_STRING // problematic - collides with doctest::detail::type_to_string<> :D
#define test_case_template              DOCTEST_TEST_CASE_TEMPLATE
#define test_case_template_define       DOCTEST_TEST_CASE_TEMPLATE_DEFINE
#define test_case_template_instantiate  DOCTEST_TEST_CASE_TEMPLATE_INSTANTIATE
#define subcase                         DOCTEST_SUBCASE
#define test_suite                      DOCTEST_TEST_SUITE
#define test_suite_begin                DOCTEST_TEST_SUITE_BEGIN
#define test_suite_end                  DOCTEST_TEST_SUITE_END
#define register_exception_translator   DOCTEST_REGISTER_EXCEPTION_TRANSLATOR
//#define info                            DOCTEST_INFO
#define capture                         DOCTEST_CAPTURE
#define add_message_at                  DOCTEST_ADD_MESSAGE_AT
#define add_fail_check_at               DOCTEST_ADD_FAIL_CHECK_AT
#define add_fail_at                     DOCTEST_ADD_FAIL_AT
//#define message                         DOCTEST_MESSAGE
//#define fail_check                      DOCTEST_FAIL_CHECK
//#define fail                            DOCTEST_FAIL
#define to_lvalue                       DOCTEST_TO_LVALUE

#define warn_eq                         DOCTEST_FAST_WARN_EQ
#define check_eq                        DOCTEST_FAST_CHECK_EQ
#define require_eq                      DOCTEST_FAST_REQUIRE_EQ
#define warn_ne                         DOCTEST_FAST_WARN_NE
#define check_ne                        DOCTEST_FAST_CHECK_NE
#define require_ne                      DOCTEST_FAST_REQUIRE_NE
#define warn_gt                         DOCTEST_FAST_WARN_GT
#define check_gt                        DOCTEST_FAST_CHECK_GT
#define require_gt                      DOCTEST_FAST_REQUIRE_GT
#define warn_lt                         DOCTEST_FAST_WARN_LT
#define check_lt                        DOCTEST_FAST_CHECK_LT
#define require_lt                      DOCTEST_FAST_REQUIRE_LT
#define warn_ge                         DOCTEST_FAST_WARN_GE
#define check_ge                        DOCTEST_FAST_CHECK_GE
#define require_ge                      DOCTEST_FAST_REQUIRE_GE
#define warn_le                         DOCTEST_FAST_WARN_LE
#define check_le                        DOCTEST_FAST_CHECK_LE
#define require_le                      DOCTEST_FAST_REQUIRE_LE
#define warn_un                         DOCTEST_FAST_WARN_UNARY
#define check_un                        DOCTEST_FAST_CHECK_UNARY
#define require_un                      DOCTEST_FAST_REQUIRE_UNARY
#define warn_not                        DOCTEST_FAST_WARN_UNARY_FALSE
#define check_not                       DOCTEST_FAST_CHECK_UNARY_FALSE
#define require_not                     DOCTEST_FAST_REQUIRE_UNARY_FALSE

// clang-format on

#endif // MY_PROXY_MACROS
