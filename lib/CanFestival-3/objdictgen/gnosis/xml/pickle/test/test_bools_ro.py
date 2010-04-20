
# read-only version of bool test.
# show that bools are converted to the "best" value, depending
# on Python version being used. --fpm

import gnosis.xml.pickle as xmp
from gnosis.xml.pickle.util import setVerbose, setParser, setParanoia
from funcs import set_parser, unlink
import gnosis.pyconfig as pyconfig

from types import *

# standard test harness setup
set_parser()

# allow unpickler to load my classes
setParanoia(0)

# NOTE: The XML below was created by running test_bool.py with
# Python >= 2.3 and grabbing the XML output.

x1 = """<?xml version="1.0"?>
<!DOCTYPE PyObject SYSTEM "PyObjects.dtd">
<PyObject module="__main__" class="foo" id="168690156">
<attr name="a" family="uniq" type="False" value="" />
<attr name="c" family="none" type="None" />
<attr name="b" family="uniq" type="True" value="" />
<attr name="k" family="lang" type="class" module="__main__" class="a_test_class"/>
<attr name="f" family="lang" type="function" module="__main__" class="a_test_function"/>
</PyObject>
"""

x2 = """<?xml version="1.0"?>
<!DOCTYPE PyObject SYSTEM "PyObjects.dtd">
<PyObject family="obj" type="builtin_wrapper"  class="_EmptyClass">
<attr name="__toplevel__" family="seq" type="tuple" id="169764140" >
  <item family="uniq" type="True" value="" />
  <item family="uniq" type="False" value="" />
</attr>
</PyObject>
"""

x3 = """<?xml version="1.0"?>
<!DOCTYPE PyObject SYSTEM "PyObjects.dtd">
<PyObject family="obj" type="builtin_wrapper"  class="_EmptyClass">
<attr name="__toplevel__" family="uniq" type="True" value="" />
</PyObject>
"""

# copied the data types from test_bools also
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

# If this Python doesn't have True/False, then the unpickler
# will return 1/0 instead
if not pyconfig.Have_TrueFalse():
    True = 1
    False = 0

# the tests are portable versions of those in test_bools.py

# bools inside an object
x = xmp.loads(x1)

# check it
if x.a != False or x.b != True or x.c != None or \
   x.f != a_test_function or x.k != a_test_class:
    #print x.__dict__
    raise "ERROR(1)"

# bools inside a toplevel bltin
x = xmp.loads(x2)
if x[0] != True or x[1] != False:
    raise "ERROR(2)"

# bool as toplevel
x = xmp.loads(x3)
if x != True:
    raise "ERROR(3)"

print "** OK **"

