import os
import sys
import subprocess

BIN_PATH = sys.argv[1]
BIN_DIR = os.path.dirname(BIN_PATH)
LIB_DIR = os.path.join(BIN_DIR, 'libs')


otool_p = subprocess.run(['otool', '-L', BIN_PATH], stdout=subprocess.PIPE)
otool_output = otoolp.stdout.decode('utf-8')

