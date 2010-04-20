
"""Make sure xml_pickle and xml_objectify play nicely together --dqm
Note that xml_pickle no longer has any builtin knowledge of
xml_objectify -- used to be necessary to workaround circular refs (fpm)"""

import gnosis.xml.objectify as xo
import gnosis.xml.pickle as xp
from StringIO import StringIO
import funcs

funcs.set_parser()
    
xml = '''<?xml version="1.0"?>
<!DOCTYPE Spam SYSTEM "spam.dtd" >
<Spam>
  <Eggs>Some text about eggs.</Eggs>
  <MoreSpam>Ode to Spam</MoreSpam>
</Spam>
'''

fh = StringIO(xml)
o = xo.make_instance(xml)
s = xp.dumps(o)
#print "---------- xml_objectify'd object, xml_pickle'd ----------"
#print s
o2 = xp.loads(s)
#print "---------- object after the pickle/unpickle cycle --------"
#print xp.dumps(o2)

if o.Eggs.PCDATA != o2.Eggs.PCDATA or \
   o.MoreSpam.PCDATA != o2.MoreSpam.PCDATA:
    raise "ERROR(1)"

print "** OK **"






