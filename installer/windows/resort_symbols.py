#!python

import errno
import os
import sys

def mkdir_p(path):
    try:
        os.makedirs(path)
    except OSError as exc: # Python >2.5
        if exc.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else: raise

symbols_dir = sys.argv[1]

for root, subdirs, files in os.walk(symbols_dir):
    for filename in files:
        file_path = os.path.join(root, filename)
        with open (file_path, 'rb') as f:
            first_line = f.readline()
            hash = first_line.split()[3];
        dest_sym_dir = os.path.join(symbols_dir, os.path.basename(os.path.splitext(file_path)[0]), hash)
        mkdir_p(dest_sym_dir)
        os.rename(file_path, os.path.join(dest_sym_dir, filename))
