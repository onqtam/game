#!/usr/bin/python

import os
import sys
import argparse
import multiprocessing
import subprocess

sys.path.append('./scripts')

from python import *

generators = ['msvc', 'gcc', 'js', 'nj']
if not isWindows():
    generators = ['gcc']

parser = argparse.ArgumentParser()
parser.add_argument("-s",                   action = "store_true",                      help = "sets up the repository (submodules, .zip files)")
parser.add_argument("--setup-emscripten",   action = "store_true",                      help = "sets up emscripten (for windows?)")
parser.add_argument("--re",                 action = "store_true",                      help = "run the emscripten .html through emrun")
parser.add_argument("--fm",                 action = "store_true",                      help = "clang-formats everything in the git diff")
parser.add_argument("-g",                   choices = generators,                       help = "runs cmake for one of the generators")
parser.add_argument("-b",                   choices = generators,                       help = "builds the generated build files")
parser.add_argument("-r",                   choices = generators,                       help = "runs the build")
parser.add_argument("-c",                   choices = ['debug', 'release'], default = "release", help = "config to build/generate for")
parser.add_argument("--cmake", nargs = argparse.REMAINDER, metavar="f", default = [],   help = "flags for cmake when generating - should be last")
args = parser.parse_args()

if args.s or args.setup_emscripten:
    command = ['python', 'scripts/python/setup.py']
    if args.setup_emscripten:
        command.append('--setup-emscripten')
    subprocess.check_call(command)

if args.g:
    # forward the --cmake flags if present
    command = ['python', 'scripts/python/run_cmake.py', '-g', args.g, '-c', args.c]
    if args.cmake:
        command.append('--cmake')
        command.extend(args.cmake)
    subprocess.check_call(command)

if args.b:
    subprocess.check_call(['python', 'scripts/python/build.py', '-b', args.b, '-c', args.c])

if args.r:
    my_env = os.environ.copy()
    if not isWindows():
        my_env['LSAN_OPTIONS'] = 'suppressions=scripts/other/lsan.supp'
        my_env['UBSAN_OPTIONS'] = 'suppressions=scripts/other/ubsan.supp'
        my_env['ASAN_OPTIONS'] = 'suppressions=scripts/other/asan.supp:allow_addr2line=true:check_initialization_order=true:strict_init_order=true:strict_string_checks=true:detect_odr_violation=2:detect_stack_use_after_return=true:verbosity=0'
    subprocess.Popen([os.getcwd() + '/bin/' + args.r + '/' + args.c.title() + '/game'], env = my_env)

if args.fm:
    subprocess.check_call(['python', 'scripts/python/format_modifications.py'])

if args.re:
    subprocess.Popen([os.getcwd() + '/scripts/batch/emscripten_build_run.bat', args.c.title()])

















































