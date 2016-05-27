#!/usr/bin/env python
# -*- coding: utf-8 -*-
from __future__ import print_function
import os, sys, hashlib

global ASSET_DIR,RSYNC_MOD,RSYNC_OPT

ASSET_DIR = os.path.expanduser('~/.gitbig/default')
RSYNC_MOD = None
RSYNC_OPT = None

def get_cache_path(hexdigest):
    return os.path.join(ASSET_DIR,hexdigest[:2],hexdigest[2:4],hexdigest[4:])

def read_stdin():
    return os.fdopen(sys.stdin.fileno(), 'rb').read()

def write_stdout(content):
    return os.fdopen(sys.stdout.fileno(), 'wb').write(content)

def write_bytes(fname,content):
    f=open(fname,"wb")
    f.write(content)
    f.close()

def read_bytes(fname):
    f=open(fname,"rb")
    content=f.read()
    f.close()
    return content

def rsync():
    os.system('rsync -%s "%s" %s' % (RSYNC_OPT,ASSET_DIR,RSYNC_MOD))

def store():
    content = read_stdin()
    hexdigest = hashlib.sha1(content).hexdigest()
    cache_path = get_cache_path(hexdigest)
    if not os.path.exists(cache_path):
        dirname=os.path.dirname(cache_path)
        os.system('mkdir -p "%s"' % dirname)
        write_bytes(cache_path,content)
    rsync()
    write_stdout(hexdigest)

def is_valid_hash(hex):
    if len(hex) != 40:
        return False
    try:
        int(hex, 16)
    except ValueError:
        return False
    else:
        return True

def load():
    hexdigest = read_stdin()
    if not is_valid_hash(hexdigest):
        write_stdout(hexdigest)
        return
    rsync()
    cache_path = get_cache_path(hexdigest)
    contents = read_bytes(cache_path)
    write_stdout(contents)

if __name__ == '__main__':
    import argparse
    
    parser = argparse.ArgumentParser(description='usage of gitbig') 
    parser.add_argument('-m','--mode',required=True)
    parser.add_argument('-d','--assetdir',default="~/.gitbig/default/")
    parser.add_argument('-r','--rsyncmod',required=True) 
    parser.add_argument('-o','--rsyncopt',default="avz")
    parser.add_argument('--version', action='version', version='%(prog)s 0.1') # version
    args = parser.parse_args() 

    ASSET_DIR=args.assetdir
    RSYNC_MOD=args.rsyncmod
    RSYNC_OPT=args.rsyncopt

    if args.mode == 'store':
        store()
    elif args.mode == 'load':
        load()
