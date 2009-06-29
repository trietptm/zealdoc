#!/usr/bin/python

import string

from Crypto.Cipher import XOR

def str2hex(s):
	return "".join([string.zfill(hex(ord(x))[2:], 2)
		for x in tuple(s)])
key = "atbnxv"
src = "xxxxxx"
obj = XOR.new(key)
en = obj.encrypt(src)
de = obj.decrypt(en)
en2 = '\x19\x0c\x1a\x16\x00\x0e'
print "%s" % en2

print "key is %s" % key
print "src is %s" % src
print "en is %s" % (str2hex(en))
#print "en is %s/%s" % (en, str2hex(en))
print "de is %s/%s" % (de, str2hex(de))
