
#
# basic test using only builtins   --fpm
#
# (good diagnostic when I break things :-)
#

from types import *
import gnosis.xml.pickle as xml_pickle
from gnosis.xml.pickle.util import setInBody
import funcs

funcs.set_parser()

class foo_class:
    def __init__(self):
        pass

def checkfoo(o1,o2):
    "Check that objects match (sync w/obj creation below)"
    # make sure it pulled out the correct class
    if o1.__class__ != foo_class or \
       o2.__class__ != foo_class:
        raise "ERROR(0)"

    # check data
    for attr in ['s1','s2','f','i','i2','li',
                 'j','n','d','l','tup']:
        if getattr(o1,attr) != getattr(o2,attr):
            raise "ERROR(1)"
        
### we print type+value to make sure unpickling really worked
##def printfoo(obj):
##	  print type(obj.s1), obj.s1
##	  print type(obj.s2), obj.s2
##	  print type(obj.f), obj.f
##	  print type(obj.i), obj.i, type(obj.i2), obj.i2
##	  print type(obj.li), obj.li
##	  print type(obj.j), obj.j
##	  print type(obj.n), obj.n
##	  print type(obj.d), obj.d['One'], obj.d['Two'], obj.d['Three']
##	  print type(obj.l), obj.l[0], obj.l[1], obj.l[2]
##	  print type(obj.tup), obj.tup[0], obj.tup[1], obj.tup[2]

xml_pickle.setParanoia(0)  # allow it to use our namespace

foo = foo_class()

# try putting numeric content in body (doesn't matter which
# numeric type)
setInBody(ComplexType,1)

# test both code paths

# path 1 - non-nested ('attr' nodes)
foo.s1 = "this is a \" string with a ' in it"
foo.s2 = 'this is a \' string with a " in it'
foo.f = 123.456
foo.i = 789
foo.i2 = 0 # zero was a bug in 1.0.1
foo.li = 5678L
foo.j = 12+34j
foo.n = None

# path 2 - nested ('item/key/val' nodes)
foo.d = { 'One': "First dict item",
          'Two': 222,
          'Three': 333.444 }

foo.l = []
foo.l.append( "first list" )
foo.l.append( 321 )
foo.l.append( 12.34 )

foo.tup = ("tuple", 123, 444.333)

#xml_pickle.setVerbose(1)

#print "---PRE-PICKLE---"
#printfoo(foo)

x1 = xml_pickle.XML_Pickler(foo).dumps()
#print x1

#print "---POST-PICKLE---"
bar = xml_pickle.XML_Pickler().loads(x1)
#printfoo(bar)

checkfoo(foo,bar)

#x2 = xml_pickle.XML_Pickler(bar).dumps()

#print "---XML from original---"
#print x1

#print "---XML from copy---"
#print x2

print "** OK **"





