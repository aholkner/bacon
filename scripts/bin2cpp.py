import argparse
import os.path
import zlib

def encode_data(data):
    lines = []
    for i in range(0, len(data), 16):
        bytes = data[i:i+16]
        line = '    ' + ', '.join(hex(ord(byte)) for byte in bytes) + ',\n'
        lines.append(line)
    return ''.join(lines)

def create_header(name):
    return '''// Generated with bin2cpp.py
    
#pragma once

extern unsigned int %(name)s_Length;
extern unsigned int %(name)s_CompressedLength;
extern unsigned char %(name)s_Compressed[];
''' % dict(name=name)

def create_source(name, data, header):
    compressed_data = zlib.compress(data)
    encoded_data = encode_data(compressed_data)
    return '''// Generated with bin2cpp.py
    
#include "%(header)s"

unsigned int %(name)s_Length = %(length)d;
unsigned int %(name)s_CompressedLength = %(compressed_length)d;

unsigned char %(name)s_Compressed[] =  {
%(encoded_data)s
};
''' % dict(name=name, encoded_data=encoded_data, compressed_length=len(compressed_data), length=len(data), header=header)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Convert a binary file to a .h/.cpp pair for inclusion in native code')
    parser.add_argument('--input', required=True)
    parser.add_argument('--output-cpp', required=True)
    parser.add_argument('--output-h', required=True)
    parser.add_argument('--name', required=True)
    args = parser.parse_args()

    data = open(args.input, 'rb').read()
    
    open(args.output_cpp, 'wt').write(create_source(args.name, data, os.path.basename(args.output_h)))
    open(args.output_h, 'wt').write(create_header(args.name))