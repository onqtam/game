#include "parser.hpp"

#include "parser.util.hpp"

using namespace std;

namespace
{
CXChildVisitResult VisitEnum(CXCursor cursor, CXCursor parent, CXClientData client_data) {
    if(clang_getCursorKind(cursor) == CXCursor_EnumConstantDecl) {
        string name  = Convert(clang_getCursorSpelling(cursor));
        int    value = static_cast<int>(clang_getEnumConstantDeclValue(cursor));
        reinterpret_cast<Enum::ValueList*>(client_data)->emplace_back(name, value);
    }
    return CXChildVisit_Continue;
}

Enum::ValueList GetEnumValues(const CXCursor& cursor) {
    Enum::ValueList result;

    clang_visitChildren(cursor, VisitEnum, &result);

    return result;
}
} // namespace

Enum GetEnum(CXCursor cursor) {
    Enum e(GetFile(cursor), GetFullName(cursor));
    e.Values = GetEnumValues(cursor);
    return e;
}
