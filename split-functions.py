#!/usr/bin/python

import argparse
import ast
import os
import subprocess
import sys

from tempfile import mkstemp

def get_function_info(c_file):
    functions = []
    e = os.path.join(os.path.dirname(sys.argv[0]), 'PrintFunctionInfo')
    proc = subprocess.Popen([e, c_file], stdout=subprocess.PIPE)
    out = proc.communicate()[0].strip()
    for line in out.split('\n'):
        functions.append(ast.literal_eval(line))    

    print functions
    return functions

def main():
    parser = argparse.ArgumentParser(description='Pull out functions from .c files')
    parser.add_argument('c_file', help='the .c file to work on')
    args = parser.parse_args()

    c_file_path = os.path.abspath(args.c_file)
    function_info = get_function_info(c_file_path)

    with open(c_file_path, 'rU') as c_file:
        contents = c_file.readlines()
        for index, function in enumerate(function_info):
            begin = function['begin']
            end = function['end']
            tmpfile, tmpfilename = mkstemp(dir=os.curdir)
            first_line = begin[0] - 1
            first_char = begin[1] - 1

            last_line = end[0]
            last_char = end[1]

            for i, line in enumerate(contents[first_line:last_line]):
                if i == 0 and i == last_line - first_line - 1:
                    os.write(tmpfile, line[first_char:last_char])
                elif i == 0:
                    os.write(tmpfile, line[first_char:])
                elif i == last_line - first_line - 1:
                    os.write(tmpfile, line[:last_char])
                else:
                    os.write(tmpfile, line)

            dest = os.path.splitext(c_file_path)[0]
            os.rename(tmpfilename, dest + '_' + str(index) + '.c')

if __name__ == '__main__':
    main()

