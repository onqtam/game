#include "parser.hpp"

#include <iostream>

#include <clang-c/Index.h>

#include "parser.util.hpp"

using namespace std;

namespace
{
ostream& operator<<(ostream& s, const CXString& str) {
    s << Convert(str);
    return s;
}

CXTranslationUnit Parse(CXIndex& index, const char* file, int argc, const char* const argv[]) {
    CXTranslationUnit unit;

    auto EC = clang_parseTranslationUnit2(index, file, argv, argc, nullptr, 0,
                                          CXTranslationUnit_Incomplete |
                                                  CXTranslationUnit_SkipFunctionBodies |
                                                  CXTranslationUnit_SingleFileParse,
                                          &unit);

    if(EC != CXError_Success) {
        cerr << "Unable to parse translation unit. Quitting." << endl;
        exit(-1);
    }

    auto diagnostics = clang_getNumDiagnostics(unit);
    if(diagnostics != 0) {
        //cerr << "> Diagnostics:" << endl;
        //for(int i = 0; i != diagnostics; ++i) {
        //    auto diag = clang_getDiagnostic(unit, i);
        //    cerr << "> " << clang_formatDiagnostic(diag, clang_defaultDiagnosticDisplayOptions())
        //         << endl;
        //}
    }

    return unit;
}

CXChildVisitResult GetTypesVisitor(CXCursor cursor, CXCursor parent, CXClientData client_data) {
    if(!clang_Location_isFromMainFile(clang_getCursorLocation(cursor))) {
        return CXChildVisit_Continue;
    }

    auto*                     data = reinterpret_cast<vector<unique_ptr<TypeBase>>*>(client_data);
    std::unique_ptr<TypeBase> type;
    switch(clang_getCursorKind(cursor)) {
        case CXCursor_EnumDecl: type = std::make_unique<Enum>(GetEnum(cursor)); break;
        case CXCursor_ClassDecl:
        case CXCursor_StructDecl: type = std::make_unique<Class>(GetClass(cursor)); break;
        case CXCursor_FunctionDecl: type = std::make_unique<Function>(GetFunction(cursor)); break;
        default: printf(""); break;
    }

    const string& name = type->GetFullName();
    if(type && !name.empty()
       //&& parser::IsRecursivelyPublic(cursor)
       //&& !(name.back() == ':')
       //&& !(name.substr(0, 3) == "::_")
       //&& regex_match(name, data->options->include)
       //&& !regex_match(name, data->options->exclude)
    ) {
        cout << "    " << name << endl;
        data->push_back(std::move(type));
    }

    return CXChildVisit_Recurse;
}
} // namespace

vector<unique_ptr<TypeBase>> GetTypes(const char* file, int argc, char* argv[]) {
    cout << "[codegen] parsing file " << endl << file << endl;
    vector<unique_ptr<TypeBase>> results;
    CXIndex                      index = clang_createIndex(0, 0);

    const char* const args[] = {"-x", "c++", "-std=c++17",
                                "-DATTRS(...)=__attribute__((annotate(#__VA_ARGS__)))",
                                "-DHAPI=__attribute__((visibility(\"default\")))"};

    CXTranslationUnit unit = Parse(index, file, sizeof(args) / sizeof(args[0]), args);

    auto cursor = clang_getTranslationUnitCursor(unit);

    clang_visitChildren(cursor, GetTypesVisitor, &results);

    clang_disposeTranslationUnit(unit);
    clang_disposeIndex(index);
    return results;
}
