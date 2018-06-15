#include "parser.util.hpp"

#include <string>
#include <sstream>
#include <vector>
#include <iterator>
#include <algorithm>
#include <cctype>
#include <locale>

using namespace std;

namespace
{
void ltrim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) { return !std::isspace(ch); }));
}

void rtrim(std::string& s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) { return !std::isspace(ch); }).base(),
            s.end());
}

std::string& trim(std::string& s) {
    ltrim(s);
    rtrim(s);
    return s;
}

std::set<std::string> split_and_trim(const std::string& s) {
    std::set<std::string> out;
    std::stringstream     ss(s);
    std::string           item;
    while(std::getline(ss, item, ','))
        out.insert(trim(item));
    return out;
}
} // namespace

string Convert(const CXString& s) {
    string result = clang_getCString(s);
    clang_disposeString(s);
    return result;
}

string GetFullName(CXCursor cursor) {
    string name;
    while(clang_isDeclaration(clang_getCursorKind(cursor)) != 0) {
        string cur = Convert(clang_getCursorSpelling(cursor));
        if(name.empty()) {
            name = cur;
        } else {
            name = cur + "::" + name;
        }
        cursor = clang_getCursorSemanticParent(cursor);
    }

    return name;
}

string GetName(const CXType& type) {
    //TODO: unfortunately, this isn't good enough. It only works as long as the
    // type is fully qualified.
    return Convert(clang_getTypeSpelling(type));
}

string GetFile(const CXCursor& cursor) {
    auto   location = clang_getCursorLocation(cursor);
    CXFile file;
    clang_getSpellingLocation(location, &file, nullptr, nullptr, nullptr);
    return Convert(clang_getFileName(file));
}

bool IsRecursivelyPublic(CXCursor cursor) {
    while(clang_isDeclaration(clang_getCursorKind(cursor)) != 0) {
        auto access = clang_getCXXAccessSpecifier(cursor);
        if(access == CX_CXXPrivate || access == CX_CXXProtected) {
            return false;
        }

        if(clang_getCursorLinkage(cursor) == CXLinkage_Internal) {
            return false;
        }

        if(clang_getCursorKind(cursor) == CXCursor_Namespace &&
           Convert(clang_getCursorSpelling(cursor)).empty()) {
            // Anonymous namespace.
            return false;
        }

        cursor = clang_getCursorSemanticParent(cursor);
    }

    return true;
}

CXChildVisitResult attr_visit(CXCursor cursor, CXCursor parent, CXClientData data) {
    if(clang_isAttribute(cursor.kind)) {
        *(CXCursor*)data = cursor;
        return CXChildVisit_Break;
    }
    return CXChildVisit_Continue;
}

set<string> GetAttributes(const CXCursor& c) {
    CXCursor attr = clang_getNullCursor();
    if(clang_visitChildren(c, attr_visit, &attr))
        return split_and_trim(Convert(clang_getCursorDisplayName(attr)));
    return {};
}
