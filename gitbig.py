#!/usr/bin/env python
# -*- coding: utf-8 -*-
from __future__ import print_function
import os, sys, hashlib
import shlex,subprocess

global ASSET_DIR,RSYNC_MOD,RSYNC_OPT,RSYNC,OPENSSL

ASSET_DIR = os.path.expanduser('~/.gitbig/default')
RSYNC_MOD = None
RSYNC_OPT = None

def get_cache_path(hexdigest):
    return os.path.join(ASSET_DIR,hexdigest[:2],hexdigest[2:4],hexdigest[4:])

def read_stdin():
    return os.fdopen(sys.stdin.fileno(), 'rb').read()

def read_stdin2():
    f=os.fdopen(sys.stdin.fileno(), 'rb')
    
    while True:
        data=f.read(1024*16)
        if len(data)!=0:
            yield data
        else:
            break

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

def rsync(hexdigest):
    f=get_cache_path(hexdigest)
    cmdline = 'rsync -%s "%s" %s' % (RSYNC_OPT,os.path.join(ASSET_DIR,f),os.path.join(RSYNC_MOD,f))
    args = shlex.split(cmdline)
    DEVNULL = open(os.devnull, 'wb')
    p=subprocess.Popen(args,stdin=None,stdout=DEVNULL,stderr=DEVNULL)
    p.wait()

def store():
    # h = hashlib.sha1()
    # content=b""
    # for _content in read_stdin2():
        # h.update(_content)
        # content+=_content
    # hexdigest = h.hexdigest()

    DEVNULL = open(os.devnull, 'wb')
    p = subprocess.Popen(['/opt/local/bin/openssl','sha1'],stdout=subprocess.PIPE,stdin=subprocess.PIPE,stderr=DEVNULL)
    content=b""
    for _content in read_stdin2():
        content+=_content
        p.stdin.write(_content)
    
    hexdigest=p.communicate()[0]
    hexdigest=hexdigest.split(" ")[1]
    p.stdin.close()
    
    cache_path = get_cache_path(hexdigest)
    if not os.path.exists(cache_path):
        dirname=os.path.dirname(cache_path)
        os.system('mkdir -p "%s"' % dirname)
        write_bytes(cache_path,content)
    elif os.path.getsize(cache_path)!=len(content):
        dirname=os.path.dirname(cache_path)
        os.system('mkdir -p "%s"' % dirname)
        write_bytes(cache_path,content)

    rsync(hexdigest)
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
    hexdigest = read_stdin().rstrip()
    if not is_valid_hash(hexdigest):
        write_stdout(hexdigest)
        return
    rsync(hexdigest)
    cache_path = get_cache_path(hexdigest)
    contents = read_bytes(cache_path)
    write_stdout(contents)

if __name__ == '__main__':
    import argparse
    
    parser = argparse.ArgumentParser(description='usage of gitbig') 
    parser.add_argument('-m','--mode',required=True)
    parser.add_argument('-d','--assetdir',default="~/.gitbig/default/")
    parser.add_argument('-r','--rsyncmod',required=True) 
    parser.add_argument('-o','--rsyncopt',default="avW")
    parser.add_argument('--rsync',default="rsync")
    parser.add_argument('--openssl',default="openssl")
    parser.add_argument('--version', action='version', version='%(prog)s 0.1') # version
    args = parser.parse_args() 

    ASSET_DIR=args.assetdir
    RSYNC_MOD=args.rsyncmod
    RSYNC_OPT=args.rsyncopt
    RSYNC=args.rsync
    OPENSSL=args.openssl

    if args.mode == 'store':
        store()
    elif args.mode == 'load':
        load()

    sys.stdout.flush()
    sys.stderr.flush()
