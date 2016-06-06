#!/usr/bin/env python
import hashlib
import os,sys,subprocess

# h = hashlib.sha1()

f=os.fdopen(sys.stdin.fileno(), 'rb')

# while True:
    # content=f.read(1024)
    # h.update(content)
    # if len(content)==0:
        # break

# print h.hexdigest()

p = subprocess.Popen(['openssl','sha1'],stdout=subprocess.PIPE,stdin=subprocess.PIPE)

while True:
    content=f.read(1024*16)
    if len(content)==0:
        break
    else:
        p.stdin.write(content)

hexdigest=p.communicate()[0]
hexdigest=hexdigest.split(" ")[1]
print hexdigest
p.stdin.close()

