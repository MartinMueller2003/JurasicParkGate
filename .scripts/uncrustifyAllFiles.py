#!/usr/bin/env python3

import hashlib
import os
import filecmp
import shutil

Import("env")
# importing the hashlib module

f = open("./MyEnv.txt", "w")
f.write(env.Dump())
f.close()

PROJECT_DIR = env['PROJECT_DIR']
PIOENV = env['PIOENV']
BOARD = env['BOARD']
PROGNAME = env['PROGNAME']
BOARD_MCU = env['BOARD_MCU']
SRC_DIR = PROJECT_DIR + "/src"

# print("PROJECT_DIR = " + PROJECT_DIR)
# print("PIOENV = " + PIOENV)
# print("BOARD = " + BOARD)
# print("PROGNAME = " + PROGNAME)
# print("BOARD_MCU = " + BOARD_MCU)
# print("SRC_DIR = " + SRC_DIR)

ExtensionsOfInterest = ('.h', '.hpp', '.c', '.cpp')


def hash_file(filename):
    """"This function returns the SHA-1 hash
    of the file passed into it"""

    # make a hash object
    h = hashlib.sha1()

    # open file for reading in binary mode
    with open(filename, 'rb') as file:

        # loop till the end of the file
        chunk = 0
        while chunk != b'':
            # read only 1024 bytes at a time
            chunk = file.read(1024)
            h.update(chunk)

    # return the hex representation of digest
    return h.hexdigest()


def ProcessDirectory(DirPath):
    # print("Display Files for: '" + DirPath + "'")
    for file in os.scandir(DirPath):

        if file.is_dir():
            ProcessDirectory(file.path)

        elif file.is_file():

            if file.path.endswith(ExtensionsOfInterest):
                SRC_FILE_NAME = file.path
                DST_FILE_NAME = SRC_FILE_NAME + ".txt"
                CRT_FILE_NAME = SRC_FILE_NAME + ".uncrustify"
                COMMAND = "uncrustify -q" + " -c " + PROJECT_DIR + \
                    "/uncrustify.cfg -f " + SRC_FILE_NAME + " > " + DST_FILE_NAME
                # print("SRC_FILE_NAME : '" + SRC_FILE_NAME + "'")
                # print("DST_FILE_NAME : '" + DST_FILE_NAME + "'")
                # print("COMMAND: '" + COMMAND + "'")
                os.system(COMMAND)
                # result = filecmp.cmp(SRC_FILE_NAME, DST_FILE_NAME, shallow=False)
                result = hash_file(SRC_FILE_NAME) == hash_file(DST_FILE_NAME)
                # print(result)

                if(False == result):

                    originalFileSize = os.path.getsize(SRC_FILE_NAME)
                    modifiedFileSize = os.path.getsize(DST_FILE_NAME)
                    if((originalFileSize/2) < modifiedFileSize):
                        print("Copy File: " + DST_FILE_NAME)
                        shutil.copyfile(DST_FILE_NAME, SRC_FILE_NAME)
                if os.path.exists(DST_FILE_NAME):
                    os.remove(DST_FILE_NAME)
                if os.path.exists(CRT_FILE_NAME):
                    os.remove(CRT_FILE_NAME)


def command_uncrustify(*args, **kwargs):
    exeName = shutil.which('uncrustify')
    if(exeName == None):
        print("uncrustify command not found")
    else:
        print("exeName: '" + exeName + "'")
        ProcessDirectory(SRC_DIR)


env.AddCustomTarget(
    name="uncrustify",
    dependencies=None,
    actions=[
        command_uncrustify
    ],
    title="Uncrustify Files",
    description="Execute uncrustify on all source files"
)
