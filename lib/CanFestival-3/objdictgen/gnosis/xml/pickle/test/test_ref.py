"Demonstrate what happens with/without DEEPCOPY  --fpm"

import gnosis.xml.pickle as xml_pickle
from gnosis.xml.pickle.util import setParanoia, setDeepCopy
from UserList import UserList
import sys
import funcs

funcs.set_parser()

a = (1,2,3)
b = [4,5,6]
c = {'a':1,'b':2,'c':3,'d':[100,200,300]}
dd = c['d'] # make sure subitems get refchecked
uu = UserList([10,11,12])

u = UserList([[uu,c,b,a],[a,b,c,uu],[c,a,b,uu],dd])
#print u

# allow xml_pickle to read our namespace
setParanoia(0)

# by default, with references
x1 = xml_pickle.dumps(u)
#print x
#del u

g = xml_pickle.loads(x1)
#print g

if u != g:
    raise "ERROR(1)"

# next, using DEEPCOPY
#print "------ DEEP COPY ------"
setDeepCopy(1)
x2 = xml_pickle.dumps(g)
#print x
#del g

z = xml_pickle.loads(x2)

# deepcopy version should be significantly larger
if (len(x2) - len(x1)) < 1000:
    raise "ERROR(2)"

if z != g:
    raise "ERROR(3)"

print "** OK **"


