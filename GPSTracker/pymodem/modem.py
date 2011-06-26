#!/usr/bin/env python

from serial import Serial
import time
import getopt, sys
import os

def hexit( text ):
	h = []

	for c in text:
		h.append( "%02X" % (ord(c)) )

	return " ".join(h)

def print_pack( pack ):
	ID = ord(pack[1]) << 8 | ord(pack[0])
	print "%04x (%02d) %s" %  (ID, ord(pack[2]), hexit(pack[3:]))

serial = Serial( "/dev/ttyACM1", 921600, timeout = 1, rtscts=0, dsrdtr=0 )

serial.flushInput()
serial.flushOutput()

#echo test
serial.write("echo test ... OK\n")
print serial.readline()

#send message
ID = 0x1234
data = 200*"j"

data_out = "%c%c%c%s" % (chr(ID & 0xff), chr(ID >> 8), chr(len(data)), data)
print_pack(data_out)
serial.write("s%s" % data_out)

tmp = serial.read(3)
if len(tmp) != 3:
	raise Exception("No answer")

data_in = []
data_in[0:] = tmp

data_in[3:] = serial.read(ord(tmp[2]))
if len(data_in) != ord(tmp[0] + 3):
	raise Exception("Bad packet - data expected")

print_pack(data_in)



serial.close()


