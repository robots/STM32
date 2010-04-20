"Test handling of Unicode strings and plain Python strings"

from gnosis.xml.pickle import loads,dumps
from gnosis.xml.pickle.util import setInBody
from types import StringType, UnicodeType
import funcs

funcs.set_parser()

#-- Create some unicode and python strings (and an object that contains them)
ustring = u"Alef: %s, Omega: %s" % (unichr(1488), unichr(969))
pstring = "Only US-ASCII characters"
estring = "Only US-ASCII with line breaks\n\tthat was a tab"
class C:
    def __init__(self, ustring, pstring, estring):
        self.ustring = ustring
        self.pstring = pstring
        self.estring = estring
o = C(ustring, pstring, estring)

#-- Try standard pickling cycle (default setInBody() settings)
#print '\n------------* Pickle with Python and Unicode strings *------------------'
xml = dumps(o)
#print xml,
#print '------------* Restored attributes from different strings *--------------'
o2 = loads(xml)
# check types explicitly, since comparison will coerce types
if not isinstance(o2.ustring,UnicodeType):
    raise "AAGH! Didn't get UnicodeType"
if not isinstance(o2.pstring,StringType):
    raise "AAGH! Didn't get StringType for pstring"
if not isinstance(o2.estring,StringType):
    raise "AAGH! Didn't get StringType for estring"

#print "UNICODE:", `o2.ustring`, type(o2.ustring)
#print "PLAIN:  ", o2.pstring, type(o2.pstring)
#print "ESCAPED:", o2.estring, type(o2.estring)

if o.ustring != o2.ustring or \
   o.pstring != o2.pstring or \
   o.estring != o2.estring:
    raise "ERROR(1)"

#-- Pickle with Python strings in body
#print '\n------------* Pickle with Python strings in body *----------------------'
setInBody(StringType, 1)
xml = dumps(o)
#print xml,
#print '------------* Restored attributes from different strings *--------------'
o2 = loads(xml)
# check types explicitly, since comparison will coerce types
if not isinstance(o2.ustring,UnicodeType):
    raise "AAGH! Didn't get UnicodeType"
if not isinstance(o2.pstring,StringType):
    raise "AAGH! Didn't get StringType for pstring"
if not isinstance(o2.estring,StringType):
    raise "AAGH! Didn't get StringType for estring"

#print "UNICODE:", `o2.ustring`, type(o2.ustring)
#print "PLAIN:  ", o2.pstring, type(o2.pstring)
#print "ESCAPED:", o2.estring, type(o2.estring)

if o.ustring != o2.ustring or \
   o.pstring != o2.pstring or \
   o.estring != o2.estring:
    raise "ERROR(1)"

#-- Pickle with Unicode strings in attributes (FAIL)
#print '\n------------* Pickle with Unicode strings in XML attrs *----------------'
setInBody(UnicodeType, 0)
try:
    xml = dumps(o)
    raise "FAIL: We should not be allowed to put Unicode in attrs"
except TypeError:
    #print "As intended, a TypeError is encountered putting Unicode in attrs"
    pass	

print "** OK **"
