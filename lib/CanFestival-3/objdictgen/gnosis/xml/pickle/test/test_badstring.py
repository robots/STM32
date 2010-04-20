
# gnosis 1.1.1 can't correctly pickle & restore certain string & unicode
# values. gnosis 1.1.2 adds checks for those -- test that
# the checks are working.

# frankm@hiwaay.net

import gnosis.xml.pickle as xml_pickle
from gnosis.xml.pickle.util import setInBody
from types import *

class Foo:
    def __init__(self,s):
        self.s = s

class WeirdUni(unicode):
    def __init__(self,s):
        unicode.__init__(self,s)

# show that I didn't screw up normal strings
f = Foo('OK')
x = xml_pickle.dumps(f)
o = xml_pickle.loads(x)
print o.s, type(o.s)

f = Foo(u'OK')
x = xml_pickle.dumps(f)
o = xml_pickle.loads(x)
print o.s, type(o.s)

f = Foo(WeirdUni(u'OK'))
x = xml_pickle.dumps(f)
o = xml_pickle.loads(x)
print o.s, type(o.s)

# pickler should catch all these unpickleable cases.
# (to be fixed in gnosis 1.2.x)

try:
    # Unicode string that contains our special string escape    
    f = Foo(u'\xbb\xbbABC\xab\xab')
    x = xml_pickle.dumps(f)
    print "************* ERROR *************"
except Exception,exc:
    print "OK  <%s>" % str(exc)
    
try:
    # Unicode string that contains our special string escape    
    f = Foo(WeirdUni(u'\xbb\xbbABC\xab\xab'))
    x = xml_pickle.dumps(f)
    print "************* ERROR *************"
except Exception,exc:
    print "OK  <%s>" % str(exc)
        
try:
    # Unicode string that contains our special string escape    
    f = Foo({'a':u'\xbb\xbbABC\xab\xab'})
    x = xml_pickle.dumps(f)
    print "************* ERROR *************"
except Exception,exc:
    print "OK  <%s>" % str(exc)
        
try:
    # Unicode string that contains our special string escape
    f = Foo({'a':WeirdUni(u'\xbb\xbbABC\xab\xab')})
    x = xml_pickle.dumps(f)
    print "************* ERROR *************"
except Exception,exc:
    print "OK  <%s>" % str(exc)
    
try:
    # illegal Unicode value for an XML file 
    f = Foo(u'\ud800')
    x = xml_pickle.dumps(f)
    print "************* ERROR *************"
except Exception,exc:
    print "OK  <%s>" % str(exc)
    
try:
    # illegal Unicode value for an XML file
    f = Foo(WeirdUni(u'\ud800'))
    x = xml_pickle.dumps(f)
    print "************* ERROR *************"
except Exception,exc:
    print "OK  <%s>" % str(exc)
    
try:
    # safe_content assumes it can always convert the string
    # to unicode, which isn't true
    # ex: pickling a UTF-8 encoded value
    setInBody(StringType, 1)
    f = Foo('\xed\xa0\x80')
    x = xml_pickle.dumps(f)
    print "************* ERROR *************"
except Exception,exc:
    print "OK  <%s>" % str(exc)
    



