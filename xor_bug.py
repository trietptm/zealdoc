#!/usr/bin/python

import string

from Crypto.Cipher import XOR

def str2hex(s):
	return "".join([string.zfill(hex(ord(x))[2:], 2)
		for x in tuple(s)])
key = "atbgnxv"
src = "xxxxxxx"
obj = XOR.new(key)
en = obj.encrypt(src)
de = obj.decrypt(en)

print "key is %s" % key
print "src is %s" % src
print "en is %s/%s" % (en, str2hex(en))
print "de is %s/%s" % (de, str2hex(de))
