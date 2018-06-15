#include "parser.hpp"

#include "parser.util.hpp"

using namespace std;

Function GetFunction(CXCursor cursor) {
    Function f(GetFile(cursor), GetFullName(cursor));
    auto     type = clang_getCursorType(cursor);

    f.Name       = Convert(clang_getCursorSpelling(cursor));
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
