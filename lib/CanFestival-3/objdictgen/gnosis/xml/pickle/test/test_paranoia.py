"Test that the PARANOIA setting works --fpm"

import gnosis.xml.pickle as xml_pickle
from gnosis.xml.pickle.util import \
     setParanoia, getParanoia, add_class_to_store
import funcs

funcs.set_parser()

ud_xml = """<?xml version="1.0"?>
<!DOCTYPE PyObject SYSTEM "PyObjects.dtd">
<PyObject module="UserDict" class="UserDict" id="136115060">
<attr name="data" type="dict" id="136066676">
  <entry>
    <key type="string" value="One" />
    <val type="numeric" value="1" />
  </entry>
  <entry>
    <key type="string" value="Two" />
    <val type="numeric" value="2" />
  </entry>
</attr>
</PyObject>
"""

COUNTER = 0
def incOK():
    global COUNTER
    COUNTER += 1
    
x = xml_pickle.XML_Pickler()

# first case, don't allow xml_pickle to invent classes
setParanoia(2)

try:
    # should not be able to load at all
    ud = x.loads(ud_xml)
    raise "FAILED 1!!"
except:
    incOK()

# second, only allow classes from the xml_pickle namespace,
# or on-the-fly
setParanoia(1)

ud = x.loads(ud_xml)

try:
    # ud should exist, but as data only
    a = ud['Two']
    raise "FAILED 2!!"
except:
    incOK()

# next, let xml_pickle import the class directly
setParanoia(-1)

ud = x.loads(ud_xml)

try:
    # ud should have full functionality
    i = ud['Two']
    incOK()
except:
    raise "FAILED 3!!"

# now it should fail again (to prove that the manual
# import didn't "stick" in xml_pickle's namespace)
setParanoia(2)

try:
    ud = x.loads(ud_xml)
    raise "FAILED 4!!"
except:
    incOK()

# let xml_pickle use our namespace
from UserDict import UserDict

setParanoia(0)
ud = x.loads(ud_xml)

try:
    # ud should have full functionality
    i = ud['One']
    incOK()
except:
    raise "FAILED 5!!"

# once again, show it fails, so we haven't corrupted
# the xml_pickle namespace
setParanoia(2)

try:
    ud = x.loads(ud_xml)
    raise "FAILED 6!!"
except:
    incOK()

# final case, stick class in xml_pickle namespace,
# with the twist of substituting a derived class

class MyDict(UserDict): pass

add_class_to_store("UserDict", MyDict)
setParanoia(1)
ud = x.loads(ud_xml)

try:
    # did it REALLY use MyDict?
    if ud.__class__.__name__ != "MyDict":
        raise "FAILED 7!!"
    else:
        # ud should have full functionality
        i = ud['One']
        incOK()
except:
    raise "FAILED 8!!"

if COUNTER != 7:
    raise "FAILED 9!!"

print "** OK **"

    
