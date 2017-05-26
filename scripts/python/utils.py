import os
import datetime
import zipfile
import urllib
import sys
from colorama import *

# inits colorama
init()

# returns true for Windows as an OS
def isWindows():
    return os.name == "nt"

# Returns true if the directory did not exist before
def make_dir(d):
    if not os.path.exists(d):
        os.makedirs(d)
        return True
    return False

# Prints colored text surrounded by square brackets with a time stamp before it
def msg(text, color_in = "cyan", indent = 0):
    color_in = color_in.upper()
    color = Fore.RESET
    
    if color_in == "BLACK": color = Fore.BLACK
    if color_in == "RED": color = Fore.RED
    if color_in == "GREEN": color = Fore.GREEN
    if color_in == "YELLOW": color = Fore.YELLOW
    if color_in == "BLUE": color = Fore.BLUE
    if color_in == "MAGENTA": color = Fore.MAGENTA
    if color_in == "CYAN": color = Fore.CYAN
    if color_in == "WHITE": color = Fore.WHITE
    
    now = datetime.datetime.now()
    text = "[" + str(datetime.time(now.hour, now.minute, now.second)) + "] [" + text + "]"
    
    print(color + '\t' * indent + "%s" % str(text) + Fore.RESET + Back.RESET + Style.RESET_ALL)

# Used for printing messages if the env var is set
def trace(text):
    envVar = os.environ.get('TRACE_MSG')
    if envVar and int(envVar) == 1:
        msg("TRACE] [" + str(text), "RED")

# Extracts a zip file and prints a percentage of the progress
def extractZip(zip, outdir = "./"):
    z = zipfile.ZipFile(zip, "r")
    uncompress_size = sum((file.file_size for file in z.infolist()))
    extracted_size = 0
    for file in z.infolist():
        extracted_size += file.file_size
        print("%s %%\r" % (extracted_size * 100/uncompress_size)),
        z.extract(file, outdir)
    print("100")

# Downloads a file
def downloadFile(url, dest):
    msg("downloading " + dest)
    try:
        file = urllib.FancyURLopener()
        file.retrieve(url, dest)
    except:
        msg("could not download " + dest, "RED")
        sys.exit(1)

# using a .zip because python 2.7 cant extract from .7z
def downloadAndExtractZip(url, zip_name, extract_dir):
    downloadFile(url, zip_name)
    msg("extracting " + zip_name)
    extractZip(zip_name, extract_dir)
