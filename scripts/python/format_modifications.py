#!/usr/bin/python

import os
from utils import *

# change to the directory where this .py file resides
os.chdir(os.path.dirname(os.path.realpath(__file__)))

msg("clang-formatting all git modifications")

os.chdir("../../")

# calling "clang-format-diff.py" like that may be non portable - tested on windows only
os.system('git diff -U0 HEAD^ | "C:/Program Files (x86)/LLVM/share/clang/clang-format-diff.py" -i -p1')
