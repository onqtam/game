#!/usr/bin/python

import os
import argparse
import sys
import multiprocessing
import subprocess

from utils import *

parser = argparse.ArgumentParser()
parser.add_argument("--setup-emscripten",       action = "store_true",              help = "sets up emscripten (for windows?)")
args = parser.parse_args()

# change to the directory where this .py file resides
os.chdir(os.path.dirname(os.path.realpath(__file__)))

# go to root
os.chdir("../../")

if args.setup_emscripten:
    if make_dir('emscripten'):
        msg("setting up emscripten")
        os.chdir('emscripten')
        try:
            msg("downloading emscripten")
            urllib.FancyURLopener().retrieve('https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-portable-64bit.zip', 'emscripten.zip')
        except:
            msg("could not download emscripten archive!", "RED")
            sys.exit(1)
        msg("extracting emscripten zip")
        extractZip('emscripten.zip', "./")
        msg("updating emscripten")
        subprocess.check_call([os.getcwd() + '/emsdk.bat', 'update'])
        msg("installing emscripten")
        subprocess.check_call([os.getcwd() + '/emsdk.bat', 'install', 'latest'])
        subprocess.check_call([os.getcwd() + '/emsdk.bat', 'activate', 'latest'])

    sys.exit(0)

msg("updating submodules")

os.system("git submodule update --init --recursive")

make_dir("tools")

if isWindows() and not os.path.exists("tools/vswhere.exe"):
    url = 'https://github.com/Microsoft/vswhere/releases/download/1.0.62/vswhere.exe'
    downloadFile(url, "tools/vswhere.exe")

if isWindows() and not os.path.exists("tools/ninja.exe"):
    url = 'https://github.com/ninja-build/ninja/releases/download/v1.7.2/ninja-win.zip'
    downloadAndExtractZip(url, "tools/ninja.zip", "tools")
    os.remove("tools/ninja.zip")

boost_minor = "65"
if not os.path.exists('third_party/boost_1_' + boost_minor + '_0'):
    url = 'http://sourceforge.net/projects/boost/files/boost/1.' + boost_minor + '.0/boost_1_' + boost_minor + '_0.zip/download'
    downloadAndExtractZip(url, "third_party/boost.zip", "third_party")
    os.remove("third_party/boost.zip")

glew_ver = "2.1.0"
if not os.path.exists("third_party/glew-" + glew_ver):
    url = 'https://sourceforge.net/projects/glew/files/glew/2.1.0/glew-' + glew_ver + '.zip/download'
    downloadAndExtractZip(url, "third_party/glew.zip", "third_party")
    os.remove("third_party/glew.zip")
