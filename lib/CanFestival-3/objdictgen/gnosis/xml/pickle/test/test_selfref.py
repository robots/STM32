"""This started out as a test of toplevel-pickling of builtins, but has
turned into more of a test of pickling self-referencing objects (toplevels are
still tested, however). --fpm"""

import gnosis.xml.pickle as xml_pickle
import random, re, sys
from gnosis.xml.pickle.ext import XMLP_Mutator, XMLP_Mutated
import gnosis.xml.pickle.ext as mutate
from UserList import UserList
import funcs

funcs.set_parser()
    
class foo: pass

# so we can unpickle foo
xml_pickle.setParanoia(0)

# test the obvious self-refs
l = [1,2]
l.append(l)
#print l
x = xml_pickle.dumps(l)
#print x
g = xml_pickle.loads(x)
#print g
# check values & ref
if g[0] != l[0] or g[1] != l[1] or id(g[2]) != id(g):
    raise "ERROR(1)"

d = {'a':1}
d['b'] = d
#print d
x = xml_pickle.dumps(d)
#print x
g = xml_pickle.loads(x)
#print g
# check values & ref
if g['a'] != 1 or id(g['b']) != id(g):
    raise "ERROR(2)"

# pickle builtins as toplevel objects (the self-ref is actually
# inside the wrapper for these cases)

bltin_objs = ["abc",123,123.45,12+34j,{'A':1, 'B':2, 'C': 3},
              [1,2,'a','b'],('a','b',1,2)]
    
for o in bltin_objs:

    x = xml_pickle.dumps(o)
    o2 = xml_pickle.loads(x)

    if o != o2:
        raise "ERROR(3)"
    
##s = "abc"
##x = xml_pickle.dumps(s)
###print s
###print x
##g = xml_pickle.loads(x)
###print g
##if g != s:
##	raise "ERROR(3)"

##s = 123
##x = xml_pickle.dumps(s)
###print s
###print x
##g = xml_pickle.loads(x)
###print g
##if g != s:
##	raise "ERROR(4)"

##s = 123.45
##x = xml_pickle.dumps(s)
##print s
###print x
##g = xml_pickle.loads(x)
##print g

##s = 12+34j
##x = xml_pickle.dumps(s)
##print s
###print x
##g = xml_pickle.loads(x)
##print g

##s = {'A':1, 'B': 2, 'C': 3}
##x = xml_pickle.dumps(s)
##print s
###print x
##g = xml_pickle.loads(x)
##print g

##s = [1,2,'a','b']
##x = xml_pickle.dumps(s)
##print s
###print x
##g = xml_pickle.loads(x)
##print g

##s = ('a','b',1,2)
##x = xml_pickle.dumps(s)
##print s
###print x
##g = xml_pickle.loads(x)
##print g

# this is neat -- r first gets mutated into a wrapper
# object, then mutated again from SRE -> atom
# (in general, nested mutation doesn't work, but it does
# work at the toplevel this way)
r = re.compile('this\s*is (not)?a\npattern$')
x = xml_pickle.dumps(r)
#print r.pattern
#print x
g = xml_pickle.loads(x)
#print g.pattern
if r.pattern != g.pattern:
    raise "ERROR(4)"

# now, pickle the same objects as first-level attributes,
# just to sanity-check that we didn't break anything

for o in bltin_objs:
    f = foo()
    f.s = o
    x = xml_pickle.dumps(f)
    g = xml_pickle.loads(x)

    if g.s != f.s:
        raise "ERROR(5)"
    
##f = foo()

##f.s = "abc"
##x = xml_pickle.dumps(f)
##g = xml_pickle.loads(x)
###print g.s

##f.s = 123
##x = xml_pickle.dumps(f)
##g = xml_pickle.loads(x)
##print g.s

##f.s = 123.45
##x = xml_pickle.dumps(f)
##g = xml_pickle.loads(x)
##print g.s

##f.s = 12+34j
##x = xml_pickle.dumps(f)
##g = xml_pickle.loads(x)
##print g.s

##f.s = {'A':1, 'B': 2, 'C': 3}
##x = xml_pickle.dumps(f)
##g = xml_pickle.loads(x)
##print g.s

##f.s = [1,2,'a','b']
##x = xml_pickle.dumps(f)
##g = xml_pickle.loads(x)
##print g.s

##f.s = ('a','b',1,2)
##x = xml_pickle.dumps(f)
##g = xml_pickle.loads(x)
##print g.s

f = foo()
f.s = re.compile('this\s*is (not)?a\npattern$')
x = xml_pickle.dumps(f)
g = xml_pickle.loads(x)
#print g.s.pattern
if f.s.pattern != g.s.pattern:
    raise "ERROR(6)"

# show that toplevel classes get mutated too

# a null-mutator, just for demonstration
class foomu(XMLP_Mutator):
    def __init__(self):
        XMLP_Mutator.__init__(self,type(foo()),'foomu')

    # careful -- type(foo) == InstanceType == "everything" :-)
    def wants_obj(self,obj):
        return obj.__class__ == foo
    
    def mutate(self,obj):
        # a tricky self-ref
        obj.breakage = obj
        return XMLP_Mutated(obj)

    def unmutate(self,mobj):
        return mobj.obj

my = foomu()
mutate.add_mutator(my)

f = foo()
f.a = UserList([4,5,6])
f.b = "abc"
#print f.a,f.b
x = xml_pickle.dumps(f)
#print x
g = xml_pickle.loads(x)
#print g.a,g.b
if g.__class__ != foo or g.a != f.a or g.b != f.b:
    raise "ERROR(7)"

mutate.remove_mutator(my)

# handcoded selfrefs from dqm that caused problems (even
# with xml_pickle-0.51)

s="""<?xml version="1.0"?>
<!DOCTYPE PyObject SYSTEM "PyObjects.dtd">
<PyObject class="Spam" id="1111">
<attr name="lst" type="list" id="2222">
  <item type="PyObject" refid="1111" />
  <item type="list" refid="2222" />
</attr>
</PyObject>
"""
#print s
o2 = xml_pickle.loads(s)
#print xml_pickle.dumps(o2)

s="""<?xml version="1.0"?>
<!DOCTYPE PyObject SYSTEM "PyObjects.dtd">
<PyObject class="Spam" id="1111">
<attr name="parent" type="PyObject" refid="1111" />
</PyObject>
"""
#print s
o2 = xml_pickle.loads(s)
#print xml_pickle.dumps(o2)

# check ref
if id(o2) != id(o2.parent):
    raise "ERROR(8)"	

#
# from a bug report sent by Wolfgang Feix <wolfgang.feix@lagosoft.de>
#
class A:																
    def __init__(self, x=0):										
        self.__parent__ = None								   
        self.x = x												
                                                                                
    def setParent(self, p):											
        self.__parent__ = p										

x				= A(1)													
x.y				= A(2)													
x.y.z			= A(3)													
x.y.z.setParent(x.y)

#print "Expect: 1 2 3 2"
#print x.x,x.y.x,x.y.z.x,x.y.z.__parent__.x

s = xml_pickle.dumps(x)
p = xml_pickle.loads(s)

#print p.x,p.y.x,p.y.z.x,p.y.z.__parent__.x

if [x.x,x.y.x,x.y.z.x,x.y.z.__parent__.x] != \
   [p.x,p.y.x,p.y.z.x,p.y.z.__parent__.x]:
    raise "ERROR(9)"

print "** OK **"
