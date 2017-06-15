#!/usr/bin/python

import os
import sys
import argparse
import subprocess
import multiprocessing
from utils import *

parser = argparse.ArgumentParser()
parser.add_argument("-b", choices = ['msvc', 'gcc', 'js', 'nj'], required = True, help = "builds the generated build files")
parser.add_argument("-c", choices = ['debug', 'release'], default = "release", help = "config to build/generate for")
args = parser.parse_args()

make = "make"
if isWindows():
    make = "mingw32-make"

# change to the directory where this .py file resides
os.chdir(os.path.dirname(os.path.realpath(__file__)))

os.chdir("../../")
make_dir("build")
os.chdir("build")
gen_dir = args.b
if args.b != 'msvc':
    gen_dir = args.b + '_' + args.c
if make_dir(gen_dir):
    os.chdir("../")
    subprocess.check_call(['python', 'r', '-g', args.b, '-c', args.c])
    os.chdir("build")
os.chdir(gen_dir)

msg("building with " + gen_dir)

args.c = args.c.title() # make the first letter capital

if args.b == 'msvc':
    command = ['C:/Program Files (x86)/Microsoft Visual Studio/2017/Community/MSBuild/15.0/Bin/MSBuild.exe', 'All.sln', '/nologo', '/clp:Summary', '/p:Configuration=' + args.c, '/maxcpucount']
    if os.environ.get('CI') != None:
        command.append('/v:quiet')
    subprocess.check_call(command)

if args.b == 'gcc':
    subprocess.check_call([make, '--keep-going', '-j' + str(multiprocessing.cpu_count())])

if args.b == 'js':
    subprocess.check_call([make, '--keep-going', '-j' + str(multiprocessing.cpu_count())])

if args.b == 'nj':
    subprocess.check_call(['.\\..\\..\\scripts\\batch\\msvc_driver.bat', os.getcwd() + '/../../tools/ninja.exe'])



















































