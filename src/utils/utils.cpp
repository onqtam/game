#include "utils.h"
HARDLY_SUPPRESS_WARNINGS
#include <ppk_assert.h>
HARDLY_SUPPRESS_WARNINGS_END

#include "preprocessor.h"

using namespace std;

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>

#else // _WIN32

#include <sys/types.h>
#include <unistd.h>

#endif // _WIN32

#if defined(_MSC_VER)
// Changes the current working directory to the one of the exe
// Used to fix the CWD when started from VS
void setCWDToExePath() {
    TCHAR path[MAX_PATH];
    GetModuleFileName(nullptr, path, MAX_PATH);

    string spath = path;
    size_t pos   = spath.find_last_of("\\/");
    if(pos != std::string::npos) {
        SetCurrentDirectory(spath.substr(0, pos).c_str());
    }
}
#else  // _MSC_VER
void     setCWDToExePath() {}
#endif // _MSC_VER

namespace Utils
{
// taken from http://www.emoticode.net/c/simple-wildcard-string-compare-globbing-function.html
int wildcmp(const char* str, const char* wild) {
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
    PPK_ASSERT(pos != std::string::npos);
    return spath.substr(0, pos) + "/";
}

// template <size_t charCount>
// void strcpy_safe(char (&output)[charCount], const char* pSrc) {
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

static ppk::assert::implementation::AssertAction::AssertAction assertHandler(
        const char* file, int line, const char* function, const char* expression, int level,
        const char* message) {
    using namespace ppk::assert::implementation;

    char buf[2048];

    size_t num_written =
            snprintf(buf, HARDLY_COUNT_OF(buf),
                     "Assert failed!\nFile: %s\nLine: %d\nFunction: %s\nExpression: %s", file, line,
                     function, expression);

    if(message)
        snprintf(buf + num_written, HARDLY_COUNT_OF(buf) - num_written, "Message: %s\n", message);

    fprintf(stderr, "%s", buf);

    if(level < AssertLevel::Debug) {
        return static_cast<AssertAction::AssertAction>(0);
    } else if(AssertLevel::Debug <= level && level < AssertLevel::Error) {
#ifdef _WIN32

        // this might cause issues if there are multiple windows or threads
        int res = MessageBox(GetActiveWindow(), buf, "Assert failed! Break in the debugger?",
                             MB_YESNO | MB_ICONEXCLAMATION);

        if(res == 7)
            return static_cast<AssertAction::AssertAction>(0);
        return AssertAction::Break;
#else
        return AssertAction::Break;
#endif
    } else if(AssertLevel::Error <= level && level < AssertLevel::Fatal) {
        return AssertAction::Throw;
    }

    return AssertAction::Abort;
}

int setPPKAssertHandler() {
    ppk::assert::implementation::setAssertHandler(assertHandler);
    return 0;
}

} // namespace Utils
