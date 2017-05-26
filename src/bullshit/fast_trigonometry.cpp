// minimax approximation to arcsin on [0, 0.5625] with rel. err. ~= 1.5e-11
static inline float myAsinf(float x)
{
    float x8, x4, x2;
    x2 = x * x;
    x4 = x2 * x2;
    x8 = x4 * x4;
    // evaluate polynomial using a mix of Estrin's and Horner's scheme
    return (((4.533422e-2f * x2 - 1.122621e-2f) * x4 + (2.633428e-2f * x2 + 2.059633e-2f)) * x8 +
            (3.058204e-2f * x2 + 4.463053e-2f) * x4 + (7.500036e-2f * x2 + 1.666666e-1f)) *
               x2 * x +
           x;
}

// the famous q3 inverse sqrt
static inline float myInvSqrtf(float number)
{
    int32 i;
    float x2, y;
    const float threehalfs = 1.5f;

    x2 = number * 0.5f;
    y = number;
    i = *(long*)&y;
    i = 0x5f3759df - (i >> 1);
    y = *(float*)&i;
    y = y * (threehalfs - (x2 * y * y)); // 1st iteration

    return y;
}

static inline float mySqrtf(const float x) { return x * myInvSqrtf(x); }

// relative error < 2e-11 on [-1, 1]
static inline float myAcosf(float x)
{
    float xa, t;
    xa = fabsf(x);
    if(xa > 0.5625f)
        t = 2.0f * myAsinf(mySqrtf(0.5f * (1.0f - xa)));
    else
        t = 1.570796f - myAsinf(xa);
    // arccos(-x) = pi - arccos(x)
    return (x < 0.0f) ? (3.1415926535897932f - t) : t;
}

static inline float myAtan2f(float y, float x)
{
    const float ONEQTR_PI = M_PI / 4.0f;
    const float THRQTR_PI = 3.0f * M_PI / 4.0f;
    float r, angle;
    float abs_y = fabsf(y) + 1e-10f; // kludge to prevent 0/0 condition
    if(x < 0.0f) {
        r = (x + abs_y) / (abs_y - x);
        angle = THRQTR_PI;
    } else {
        r = (x - abs_y) / (x + abs_y);
        angle = ONEQTR_PI;
    }
    angle += (0.1963f * r * r - 0.9817f) * r;

    return (y < 0.0f ? -angle : angle); // negate if in quad III or IV
}