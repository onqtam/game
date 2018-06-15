#include "parser.hpp"
#include "parser.util.hpp"

using namespace std;

namespace
{
Function GetMethodFromCursor(CXCursor cursor) {
    auto type = clang_getCursorType(cursor);

    Function f(GetFile(cursor), GetFullName(cursor));
    f.Name       = Convert(clang_getCursorSpelling(cursor));
    f.Attributes = GetAttributes(cursor);
    int num_args = clang_Cursor_getNumArguments(cursor);
    for(int i = 0; i < num_args; ++i) {
        auto        arg_cursor = clang_Cursor_getArgument(cursor, i);
        NamedObject arg;
        arg.Name = Convert(clang_getCursorSpelling(arg_cursor));
        if(arg.Name.empty()) {
            arg.Name = "nameless";
        }
        auto arg_type = clang_getArgType(type, i);
        arg.Type      = GetName(arg_type);
        f.Arguments.push_back(arg);
    }

    f.ReturnType = GetName(clang_getResultType(type));
    return f;
}

NamedObject GetFieldFromCursor(CXCursor cursor) {
    NamedObject field;
    field.Name       = Convert(clang_getCursorSpelling(cursor));
    field.Type       = GetName(clang_getCursorType(cursor));
    field.Attributes = GetAttributes(cursor);
    return field;
}

CXChildVisitResult VisitClass(CXCursor cursor, CXCursor parent, CXClientData client_data) {
    auto* c = reinterpret_cast<Class*>(client_data);
    //if (clang_getCXXAccessSpecifier(cursor) == CX_CXXPublic)
    {
        switch(clang_getCursorKind(cursor)) {
            case CXCursor_CXXMethod:
                if(clang_CXXMethod_isStatic(cursor) != 0)
                    c->StaticMethods.push_back(GetMethodFromCursor(cursor));
                else
                    c->Methods.push_back(GetMethodFromCursor(cursor));
                break;
            case CXCursor_FieldDecl: c->Fields.push_back(GetFieldFromCursor(cursor)); break;
            case CXCursor_VarDecl: c->StaticFields.push_back(GetFieldFromCursor(cursor)); break;
            default: break;
        }
    }
    return CXChildVisit_Continue;
}
} // namespace

Class GetClass(CXCursor cursor) {
    Class c(GetFile(cursor), GetFullName(cursor));
    c.Attributes = GetAttributes(cursor);
    clang_visitChildren(cursor, VisitClass, &c);
    return c;
}
