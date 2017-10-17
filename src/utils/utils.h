#pragma once

namespace Utils
{

template <typename T>
T Min(T a, T b) {
    return (a < b) ? a : b;
}

template <typename T>
T Max(T a, T b) {
    return (a > b) ? a : b;
}

template <class T>
T clamp(T x, T low, T high) {
    if(x > high)
        return high;
    if(x > low)
        return x;
    return low;
}

template <class T>
T sign(T x) {
    if(x < T(0))
        return T(-1);
    if(x > T(0))
        return T(1);
    return T(0);
}

template <typename T>
T lerp(T x, T low, T high) {
    return low * (T(1) - x) + x * high;
}

// wildcard function - can use for masking '*' for any sequence and '?' for any single char
int wildcmp(cstr str, cstr wild);

// if the first string ends with the second
bool endsWith(const std::string& fullString, const std::string& ending);

inline char* strncpy(char* destination, cstr source, size_t num) {
    auto res             = ::strncpy(destination, source, num);
    destination[num - 1] = '\0';
    return res;
}

// Returns the number of digits
uint32 numDigits(int32 v);

// Returns the number of digits - recursive for really big number
uint32 numDigits(int64 v);

// Returns the path to the current process executable
std::string getPathToExe();

// variadic hash_combine taken from here: https://stackoverflow.com/a/38140932/3162383
inline void hash_combine(std::size_t&) {}
template <typename T, typename... Rest>
inline void hash_combine(std::size_t& seed, const T& v, Rest... rest) {
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    hash_combine(seed, rest...);
}

// taken from here: https://stackoverflow.com/a/16597048/3162383
template <typename ContainerT, typename PredicateT>
void erase_if(ContainerT& items, const PredicateT& predicate) {
    for(auto it = items.begin(); it != items.end();) {
        if(predicate(*it))
            it = items.erase(it);
        else
            ++it;
    }
}

// A fast and portable version of itoa for base 10
// Its 2.5 to 6 times faster than the non standart one (faster with larger numbers)
template <typename T>
char* itoa_fast(T value, char* dst) {
    static_assert(std::is_integral<T>::value, "value for itoa_fast is not integral!");
    static_assert(sizeof(T) > 1, "value for itoa_fast is too small!");

    static const char digits[201] = "00010203040506070809"
                                    "10111213141516171819"
                                    "20212223242526272829"
                                    "30313233343536373839"
                                    "40414243444546474849"
                                    "50515253545556575859"
                                    "60616263646566676869"
                                    "70717273747576777879"
                                    "80818283848586878889"
                                    "90919293949596979899";

    if(value < 0) {
        dst[0] = '-';
        itoa_fast(-value, dst + 1);
        return dst;
    }
    uint32 const length = numDigits(value);
    dst[length]              = '\0';
    uint32 next         = length - 1;
    while(value >= 100) {
        auto const i = (value % 100) * 2;
        value /= 100;
        dst[next]     = digits[i + 1];
        dst[next - 1] = digits[i];
        next -= 2;
    }

    // handle last 1-2 digits
    if(value < 10) {
        dst[next] = '0' + char(value);
    } else {
        auto i        = uint32(value) * 2;
        dst[next]     = digits[i + 1];
        dst[next - 1] = digits[i];
    }
    return dst;
}

} // namespace Utils

#define HA_OFFSET(i) ((char *)nullptr + (i))
#define HA_OFFSET_OF(type, member) (&static_cast<type*>(nullptr)->member)
