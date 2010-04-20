# this example just shows the similarity between the pickle
# and xml_pickle APIs. xml_pickle offers much more functionality,
# but there is basic API compatibility (not 100% -- esp. Pickle()
# requires an open file, whereas XML_Pickler() is much more
# flexible)  --fpm

import pickle, os
import gnosis.xml.pickle as xml_pickle
from funcs import set_parser, unlink

set_parser()

class foo: pass

f = foo()
f.l = [1,2,3]
f.d = {'A':1,'B':2,'C':3}
f.t = ([4,5,6],[7,8,9],[10,11,12])

def testfoo(o1,o2):
    for attr in ['l','d','t']:
        if getattr(o1,attr) != getattr(o2,attr):
            raise "ERROR(1)"		
#
# test dump/load to/from a string
#

s = pickle.dumps(f)
x = xml_pickle.dumps(f)

g = pickle.loads(s)
testfoo(f,g)

# have to do this here, otherwise "m" gets recreated
# with an on-the-fly class that breaks regular pickle
# (as it should --- pickle shouldn't know about xml_pickle.foo)
# we could just not reuse m, but this is kind of a neat
# way to show what's going on, and how PARANOIA works
xml_pickle.setParanoia(0)

m = xml_pickle.loads(x)
testfoo(f,m)

# now test dump/load to/from a file

fh = open('aaa','w')
pickle.dump(m,fh)
fh.close()

fh = open('bbb','w')
xml_pickle.dump(m,fh)
fh.close()

fh = open('aaa','r')
g = pickle.load(fh)
fh.close()
testfoo(f,g)

fh = open('bbb','r')
g = xml_pickle.load(fh)
fh.close()
testfoo(f,g)

unlink('aaa')
unlink('bbb')

print "** OK **"

