"Test the namespace manipulation of _XO_ classes"
import sys
PARSER='EXPAT'
if len(sys.argv) > 1:
    PARSER=sys.argv[1]

from gnosis.xml.objectify import _XO_
class _XO_Eggs(_XO_):
    def __init__(self):
        self.this = 'that'
    def hello(self):
        print "Hello world"

xml_str ="""<?xml version="1.0"?>
<!DOCTYPE Spam SYSTEM "spam.dtd" >
<Spam>
  <Eggs>Some text about eggs.</Eggs>
  <MoreSpam>Ode to Spam</MoreSpam>
</Spam>"""
xml_file = open('test.xml', 'w')
xml_file.write(xml_str)
xml_file.close()

from gnosis.xml.objectify import *
import gnosis.xml.objectify

print "================================================================"
print "Create an instance, but don't manage to find our _XO_Eggs class!"
print "================================================================"
xml_object = XML_Objectify('test.xml', PARSER)
py_obj = xml_object.make_instance()
print pyobj_printer(py_obj)
try:    py_obj.Eggs.hello()
except: print "Eggs don't say hello"

print "\n================================================================"
print "OK, get our _XO_Eggs class into the xml_objectify namespace!"
print "================================================================"
gnosis.xml.objectify._XO_Eggs = _XO_Eggs
xml_object = XML_Objectify('test.xml', PARSER)
py_obj = xml_object.make_instance()
print pyobj_printer(py_obj)
try:    py_obj.Eggs.hello()
except: print "Eggs don't say hello"

print "\n================================================================"
print "Create an instance, but don't manage to find our _XO_Eggs class!"
print "================================================================"
gnosis.xml.objectify._XO_Eggs = None
xml_object = XML_Objectify('test.xml', PARSER)
py_obj = xml_object.make_instance()
print pyobj_printer(py_obj)
try:    py_obj.Eggs.hello()
except: print "Eggs don't say hello"

