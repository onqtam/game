#pragma once

#include <memory>
#include <string>
#include <vector>

#include <clang-c/Index.h>

#include "types.hpp"

std::vector<std::unique_ptr<TypeBase>> GetTypes(const char* file, int argc, char* argv[]);

Class    GetClass(CXCursor cursor);
Enum     GetEnum(CXCursor cursor);
Function GetFunction(CXCursor cursor);
