#!/usr/bin/env python

from serial import Serial
from time import ctime
import getopt, sys
import os
import array

def hexit( text ):
	h = []

	for c in text:
		h.append( "%02X" % (ord(c)) )

	return " ".join(h)

def print_pack( pack ):
	ID = ord(pack[0]) | ord(pack[1]) << 8
	print "%04x (%02d) %s" %  (ID, ord(pack[2]), hexit(pack[3:]))

serial = Serial( "/dev/ttyACM0", 921600, timeout = 1, rtscts=0, dsrdtr=0 )

serial.flushInput()
serial.flushOutput()

#echo test
serial.write("echo test ... OK\n")
print serial.readline()

#start sniffer
print "Starting sniffer"
serial.write("rr")

try:
	while True:
		tmp = []
		tmp[0:] = serial.read(1)
		if len(tmp) == 0:
			continue

# read second part of ID and length
		tmp[1:] = serial.read(2)
		if len(tmp) != 3:
			print "Malformed packet - bleh!"
			continue

# read data
		tmp[3:] = serial.read(ord(tmp[2]))
		if len(tmp) != ord(tmp[2]) + 3:
			print "Malformed packet - bleh!"
			continue

# read checksum
		chksum = serial.read(1)
		if len(chksum) == 0:
			print "Missing checksum"
			continue

		s = 0
		for c in tmp:
			s = s + ord(c)

		s = ~s & 0xff
# compare checksum
		if s != ord(chksum[0]):
			print "Bad checksum - %02x != %02x" % (s, ord(chksum[0]))
			continue

		print_pack(tmp)

except KeyboardInterrupt:
	print "Terminated..."

#terminate sniffer
serial.write("f")
serial.close()


