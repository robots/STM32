
#
# show that imported classes are unpickled OK
#

import pickle
import test_ftypes_i
import gnosis.xml.pickle as xml_pickle
import funcs

funcs.set_parser()

class foo: pass

xml_pickle.setParanoia(0)

f = foo()

f.b = test_ftypes_i.gimme_bfunc()
f.p = test_ftypes_i.gimme_pfunc()
f.f = foo

#print f.b, f.p, f.f

x = xml_pickle.dumps(f)
#print x

g = xml_pickle.loads(x)

#print g.b, g.p, g.f

# check it
for attr in ['b','p','f']:
    if getattr(f,attr) != getattr(g,attr):
        raise "ERROR(1)"

print "** OK **"

