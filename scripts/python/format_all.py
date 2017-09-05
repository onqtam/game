#!/usr/bin/python

import os
import sys
from utils import *

os.chdir("../../")

cpp_extensions = (".cpp", ".cxx", ".c++", ".cc", ".cp", ".c", ".i", ".ii", ".h", ".h++", ".hpp", ".hxx", ".hh", ".inl", ".inc", ".ipp", ".ixx", ".txx", ".tpp", ".tcc", ".tpl")

#TODO: does this go recursively in folders?

for root, dirs, files in os.walk("src"):
    for file in files:
        if file.endswith(cpp_extensions):
            os.system("clang-format -i -style=file " + root + "/" + file)

os.chdir("scripts/python")
