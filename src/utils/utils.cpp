#include "utils.h"

using namespace std;

HA_SUPPRESS_WARNINGS

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <tchar.h>

#else // _WIN32

#include <sys/types.h>
#include <unistd.h>

#endif // _WIN32

HA_SUPPRESS_WARNINGS_END

namespace Utils
{
// taken from http://www.emoticode.net/c/simple-wildcard-string-compare-globbing-function.html
int wildcmp(cstr str, cstr wild) {
    const char *cp = NULL, *mp = NULL;

    while((*str) && (*wild != '*')) {
        if((*wild != *str) && (*wild != '?')) {
            return 0;
        }
        wild++;
        str++;
    }

    while(*str) {
        if(*wild == '*') {
            if(!*++wild) {
                return 1;
            }
            mp = wild;
            cp = str + 1;
        } else if((*wild == *str) || (*wild == '?')) {
            wild++;
            str++;
        } else {
            wild = mp;
            str  = cp++;
        }
    }

    while(*wild == '*') {
        wild++;
    }
    return !*wild;
}

bool endsWith(const std::string& full, const std::string& ending) {
    if(full.length() >= ending.length()) {
        return (0 == full.compare(full.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

string getPathToExe() {
#ifdef _WIN32
    TCHAR path[MAX_PATH];
    GetModuleFileName(nullptr, path, MAX_PATH);
#else  // _WIN32
    char arg1[20];
    char path[PATH_MAX + 1] = {0};
    sprintf(arg1, "/proc/%d/exe", getpid());
    auto dummy = readlink(arg1, path, 1024);
    ((void)dummy); // to use it... because the result is annotated for static analysis
#endif // _WIN32
    string spath = path;
    size_t pos   = spath.find_last_of("\\/");
    hassert(pos != std::string::npos);
    return spath.substr(0, pos) + "/";
}

// template <size_t charCount>
// void strcpy_safe(char (&output)[charCount], cstr pSrc) {
//    strncpy(output, pSrc, charCount);
//    output[charCount - 1] = 0;
//}

uint32 numDigits(int32 v) {
    if(v < 0)
        return numDigits(-v);
    if(v < 10)
        return 1;
    if(v < 100)
        return 2;
    if(v < 1000)
        return 3;
    if(v < 100000000) {
        if(v < 1000000) {
            if(v < 10000)
                return 4;
            return 5 + (v >= 100000);
        }
        return 7 + (v >= 10000000);
    }

    return 9 + (v >= 1000000000);
}

uint32 numDigits(int64 v) {
    if(v < 0)
        return numDigits(-v);
    if(v < 10)
        return 1;
    if(v < 100)
        return 2;
    if(v < 1000)
        return 3;
    if(v < 1000000000000) {
        if(v < 100000000) {
            if(v < 1000000) {
                if(v < 10000)
                    return 4;
                return 5 + (v >= 100000);
            }
            return 7 + (v >= 10000000);
        }
        if(v < 10000000000)
            return 9 + (v >= 1000000000);
        return 11 + (v >= 100000000000);
    }
    return 12 + numDigits(v / 1000000000000);
}

glm::quat rotationBetweenVectors(glm::vec3 start, glm::vec3 dest) {
    using namespace glm;
    start = normalize(start);
    dest  = normalize(dest);

    float cosTheta = dot(start, dest);
    vec3  rotationAxis;

    if(cosTheta < -1 + 0.001f) {
        // special case when vectors in opposite directions:
        // there is no "ideal" rotation axis
        // So guess one; any will do as long as it's perpendicular to start
        rotationAxis = cross(vec3(0.0f, 0.0f, 1.0f), start);
        if(length2(rotationAxis) < 0.01f) // bad luck, they were parallel, try again!
            rotationAxis = cross(vec3(1.0f, 0.0f, 0.0f), start);

        rotationAxis = normalize(rotationAxis);
        return angleAxis(180.0f, rotationAxis);
    }

    rotationAxis = cross(start, dest);

    float s    = sqrt((1 + cosTheta) * 2);
    float invs = 1 / s;

    return quat(s * 0.5f, rotationAxis.x * invs, rotationAxis.y * invs, rotationAxis.z * invs);
}

} // namespace Utils
