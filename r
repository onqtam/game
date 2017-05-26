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
parser.add_argument("--cmake", nargs = argparse.REMAINDER, metavar="f", default = [],   help = "flags for cmake when generating - should be last")
args = parser.parse_args()

if args.s or args.setup_emscripten:
    command = ['python', 'scripts/python/setup.py']
    if args.setup_emscripten:
        command.append('--setup-emscripten')
    subprocess.check_call(command)

if args.g:
    # forward the --cmake flags if present
    command = ['python', 'scripts/python/run_cmake.py', '-g', args.g]
    if args.cmake:
        command.append('--cmake')
        command.extend(args.cmake)
    subprocess.check_call(command)

if args.b:
    subprocess.check_call(['python', 'scripts/python/build.py', '-b', args.b])

if args.fm:
    subprocess.check_call(['python', 'scripts/python/format_modifications.py'])

if args.re:
    subprocess.Popen([os.getcwd() + '/scripts/batch/emscripten_build_run.bat'])

















































