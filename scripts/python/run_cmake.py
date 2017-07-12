#!/usr/bin/python

import os
import sys
import argparse
import subprocess
from utils import *

parser = argparse.ArgumentParser()
parser.add_argument("-g", choices = ['msvc', 'gcc', 'js', 'nj'], required = True,       help = 'runs cmake for one of the generators')
parser.add_argument("-c", choices = ['debug', 'release'], default = "release", help = "config to build/generate for")
parser.add_argument("--cmake", nargs = argparse.REMAINDER, metavar="f", default = [],   help = "flags to be passed to cmake - should be last")
args = parser.parse_args()

cmake_options = []
if args.cmake:
    cmake_options.extend(args.cmake)

makefile = 'Unix Makefiles'
if isWindows():
    makefile = 'MinGW Makefiles'

# change to the directory where this .py file resides
os.chdir(os.path.dirname(os.path.realpath(__file__)))

os.chdir('../../')
make_dir('build')
os.chdir('build')
gen_dir = args.g
if args.g != 'msvc':
    gen_dir = args.g + '_' + args.c
make_dir(gen_dir)
os.chdir(gen_dir)

msg("generating build files for " + gen_dir)

args.c = args.c.title() # make the first letter capital

if args.g == 'msvc':
    command = ['cmake', '../../', '-G', "Visual Studio 15 2017 Win64"]
    command.extend(cmake_options)
    command.append('-DTOOLCHAIN=msvc')
    
    subprocess.check_call(command)

if args.g == 'gcc':
    command = ['cmake', '../../', '-G', makefile]
    command.extend(cmake_options)
    command.append('-DCMAKE_EXPORT_COMPILE_COMMANDS=ON')
    command.append('-DCMAKE_BUILD_TYPE=' + args.c)
    command.append('-DTOOLCHAIN=gcc')
    subprocess.check_call(command)

if args.g == 'js':
    command = ['.\\..\\..\\scripts\\batch\\emscripten_driver_' + gen_dir + '.bat', 'cmake', '../../', '-G', makefile]
    command.extend(cmake_options)
    command.append('-DCMAKE_EXPORT_COMPILE_COMMANDS=ON')
    command.append('-DCMAKE_BUILD_TYPE=' + args.c)
    command.append('-DTOOLCHAIN=js')
    command.append('-DEMSCRIPTEN_ROOT=' + os.getcwd() + '/../../emscripten/emscripten/1.37.16')
    command.append('-DCMAKE_TOOLCHAIN_FILE=' + os.getcwd() + '/../../emscripten/emscripten/1.37.16\cmake\Modules\Platform\Emscripten.cmake')
    command.append('-Wno-dev')
    
    subprocess.check_call(command)

if args.g == 'nj':
    command = ['.\\..\\..\\scripts\\batch\\msvc_driver.bat', 'cmake', '../../', '-G', "Ninja"]
    command.extend(cmake_options)
    command.append('-DCMAKE_MAKE_PROGRAM=' + os.getcwd() + '/../../tools/ninja.exe')
    command.append('-DCMAKE_BUILD_TYPE=' + args.c)
    command.append('-DTOOLCHAIN=msvc')
    command.append('-DCMAKE_CXX_COMPILER:FILEPATH=cl.exe')
    command.append('-DCMAKE_C_COMPILER:FILEPATH=cl.exe')
    command.append('-DCMAKE_LINKER:FILEPATH=link.exe')
    
    subprocess.check_call(command)














































