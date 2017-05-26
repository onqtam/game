#include "utils.h"
#include "utils/doctest/doctest_proxy.h"
#include "preprocessor.h"

using namespace Utils;

test_case("[utils] Basic templates") {
    check_eq(Min(1, 2), 1);
    check_eq(Min(2, 1), 1);
    check_eq(Max(1, 2), 2);
    check_eq(Max(2, 1), 2);

    check_eq(clamp(4, 2, 5), 4);
    check_eq(clamp(4, 5, 6), 5);
    check_eq(clamp(4, 2, 3), 3);

    check_eq(sign(-1), -1);
    check_eq(sign(1), 1);
    check_eq(sign(0), 0);

    check_eq(lerp(0.5, -3.0, 1.0), doctest::Approx(-1.0));
    check_eq(lerp(0.0, -3.0, 1.0), doctest::Approx(-3.0));
    check_eq(lerp(1.0, -3.0, 1.0), doctest::Approx(1.0));
}

test_case("[utils] wildcmp") {
    check_un(wildcmp("I am a string", "I am a string"));
    check_un(wildcmp("I am a string", "*"));
    check_un(wildcmp("I am a string", "**??**"));
    check_un(wildcmp("I am a string", "**?*?**"));
    check_un(wildcmp("I am a string", "?????????????"));
    check_un(wildcmp("I am a string", "I ?????????ng"));

    check_un(wildcmp("I am a string", "I am*"));
    check_un(wildcmp("I am a string", "*I am*"));
    check_un(wildcmp("I am a string", "*? am*"));
    check_un(wildcmp("I am a string", "? am*"));
    check_un(wildcmp("I am a string", "??am*"));
    check_un(wildcmp("I am a string", "?* am*"));

    check_un(wildcmp("I am a string", "*string"));
    check_un(wildcmp("I am a string", "*string*"));
    check_un(wildcmp("I am a string", "*strin?*"));
    check_un(wildcmp("I am a string", "*strin?"));
    check_un(wildcmp("I am a string", "*stri??"));
    check_un(wildcmp("I am a string", "*strin*?"));

    check_not(wildcmp("I am a string", "I  am*"));
    check_not(wildcmp("I am a string", "I??am*"));
}

test_case("[utils] endsWith") {
    check_un(endsWith("asd", "asd"));
    check_not(endsWith("asd", "asdsfa"));
    check_un(endsWith("asd", "sd"));
}

test_case("[utils] numDigits") {
    check_eq(numDigits(0), 1u);
    check_eq(numDigits(6), 1u);
    check_eq(numDigits(12), 2u);
    check_eq(numDigits(234), 3u);
    check_eq(numDigits(2355), 4u);
    check_eq(numDigits(21355), 5u);
    check_eq(numDigits(235567), 6u);
    check_eq(numDigits(2355767), 7u);
    check_eq(numDigits(23557676), 8u);
    check_eq(numDigits(235576746), 9u);
    check_eq(numDigits(2125767446), 10u);

    check_eq(numDigits(21257674464LL), 11u);
    check_eq(numDigits(212576744643LL), 12u);
    check_eq(numDigits(2125767446454LL), 13u);
    check_eq(numDigits(21257674456564LL), 14u);
    check_eq(numDigits(212534567674464LL), 15u);
    check_eq(numDigits(2125767546354464LL), 16u);
    check_eq(numDigits(21257674445656664LL), 17u);
    check_eq(numDigits(212576744656566664LL), 18u);
    check_eq(numDigits(9223372036854775807LL), 19u);
}

test_case("[utils] itoa_fast") {
#define itoa_fast_check_eq(x) check_eq(strcmp(itoa_fast(HARDLY_CAT_1(x, LL), dest), #x), 0)
#define itoa_fast_check_ne(x) check_ne(strcmp(itoa_fast(HARDLY_CAT_1(x, LL), dest), #x), 0)
    char dest[24];
    itoa_fast_check_eq(0);
    itoa_fast_check_eq(12);
    itoa_fast_check_eq(431);
    itoa_fast_check_eq(1234);
    itoa_fast_check_eq(14274);
    itoa_fast_check_eq(467836);
    itoa_fast_check_eq(4506543);
    itoa_fast_check_eq(34659300);
    itoa_fast_check_eq(900964566);
    itoa_fast_check_eq(6400964566);
    itoa_fast_check_eq(64300964566);
    itoa_fast_check_eq(645300964566);
    itoa_fast_check_eq(6400345964566);
    itoa_fast_check_eq(64009634554566);
    itoa_fast_check_eq(640096345545662);
    itoa_fast_check_eq(6400963455456645);
    itoa_fast_check_eq(640096345545664235);
    itoa_fast_check_eq(6400963455456642356);

    itoa_fast_check_ne(012);
    itoa_fast_check_ne(00004543265);
#undef itoa_fast_check_eq
#undef itoa_fast_check_ne
}
