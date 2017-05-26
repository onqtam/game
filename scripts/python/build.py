#!/usr/bin/python

import os
import sys
import argparse
import subprocess
import multiprocessing
from utils import *

parser = argparse.ArgumentParser()
parser.add_argument("-b", choices = ['msvc', 'gcc', 'js', 'nj'], required = True, help = "builds the generated build files")
args = parser.parse_args()

make = "make"
if isWindows():
    make = "mingw32-make"

# change to the directory where this .py file resides
os.chdir(os.path.dirname(os.path.realpath(__file__)))

os.chdir("../../")
make_dir("build")
os.chdir("build")
if make_dir(args.b):
    os.chdir("../")
    subprocess.check_call(['python', 'r', '-g', args.b])
    os.chdir("build")
os.chdir(args.b)

msg("building with " + args.b)

if args.b == 'msvc':
    command = ['C:/Program Files (x86)/Microsoft Visual Studio/2017/Community/MSBuild/15.0/Bin/MSBuild.exe', 'All.sln', '/nologo', '/clp:Summary', '/p:Configuration=Release', '/maxcpucount']
    if os.environ.get('CI') != None:
        command.append('/v:quiet')
    subprocess.check_call(command)

if args.b == 'gcc':
    subprocess.check_call([make, '--keep-going', '-j' + str(multiprocessing.cpu_count())])

if args.b == 'js':
    subprocess.check_call([make, '--keep-going', '-j' + str(multiprocessing.cpu_count())])

if args.b == 'nj':
    subprocess.check_call(['.\\..\\..\\scripts\\batch\\msvc_driver.bat', os.getcwd() + '/../../tools/ninja.exe'])



















































