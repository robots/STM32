
#
# Test bools (Python 2.3+)
#

import gnosis.xml.pickle as xmp
from gnosis.xml.pickle.util import setVerbose, setParanoia
from funcs import set_parser, unlink

from types import *

# set this to 1 to grab the XML output for cut&paste into test_bools_ro.py
SHOW_XML = 0

# standard test harness setup
set_parser()

class a_test_class:
    def __init__(self):
        pass

def a_test_function():
    pass

class foo:
    def __init__(self):

        self.a = False
        self.b = True
        self.c = None
        self.f = a_test_function
        self.k = a_test_class
        
# always show the family tag so I can make sure it's right
setVerbose(1)
setParanoia(0)

f = foo()

# dump an object containing bools
s = xmp.dumps(f)
if SHOW_XML:
    print s

x = xmp.loads(s)
#print "Expect False, True, None, func, class: ",x.a,x.b,x.c,x.f,x.k

# check it
for attr in ['a','b','c','f','k']:
    if getattr(f,attr) != getattr(x,attr):
        raise "ERROR(1)"
    
# dump builtin obj containing bools
s = xmp.dumps( (True,False) )
if SHOW_XML:
    print s

x = xmp.loads( s )
#print "Expect True, False: ",x[0],x[1]

# check
if x[0] != True or x[1] != False:
    raise "ERROR(2)"

# dump bool itself as toplevel obj
s = xmp.dumps( True )
if SHOW_XML:
    print s

x = xmp.loads(s)
#print "Expect True: ",x

if x != True:
    raise "ERROR(3)"

print "** OK **"
