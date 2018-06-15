#pragma once

#include <string>
#include <set>

#include <clang-c/Index.h>

std::string Convert(const CXString& s);

std::string GetFullName(CXCursor cursor);
std::string GetName(const CXType& type);
std::string GetFile(const CXCursor& cursor);

bool IsRecursivelyPublic(CXCursor cursor);

std::set<std::string> GetAttributes(const CXCursor& c);
