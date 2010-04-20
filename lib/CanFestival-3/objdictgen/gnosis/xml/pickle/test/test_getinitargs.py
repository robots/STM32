
"""
test that we get the tricky stuff correct between
__getstate__ and __getinitargs__ --fpm
"""

import pickle
import gnosis.xml.pickle as xml_pickle
import funcs
from copy import copy
from UserList import UserList

funcs.set_parser()
xml_pickle.setParanoia(0)

COUNT_INIT = 0
COUNT_GETARGS = 0

class Foo:
    # A couple of versions of Python would check for __safe_for_unpickling__
    # before allowing you to use initargs. That check has been removed, but
    # this is still needed to run this test on those version.
    # xml.pickle doesn't use __safe_for_unpickling__ at all.
    __safe_for_unpickling__ = 1
    
    def __init__(self,a,b,c,d):
        global COUNT_INIT
        COUNT_INIT += 1

        # test: unpickler should NOT restore any
        # attributes before __init__ is called
        if len(self.__dict__.keys()) != 0:
            raise "ERROR -- attrs shouldn't exist before __init__"

        # .a and .b assignments have no effect when unpickling.
        # since those values were pickled, the unpickler will
        # set them AFTER __init__ returns.
        self.a = a
        self.b = b

        # only these have an effect, since .c and .d aren't
        # pickled (see __getstate__)
        self.c = c
        self.d = d		
        
    def __getinitargs__(self):
        global COUNT_GETARGS
        COUNT_GETARGS += 1
        # on pickling, this is called to ask what args
        # should be passed to __init__() [normally __init__
        # is not called when unpickling]

        # subtle: the values I return for 'a' and 'b' below
        # are *IGNORED* since they are pickled. The only
        # values used are .c and .d, because I delete them
        # from the attrs in __getstate__ below
        return (200,'def',[4,5,6],('x','y','z'))

    def __getstate__(self):
        d = copy(self.__dict__)
        
        # delete .c and .d so they won't be pickled
        del d['c']
        del d['d']
        
        return d

# test with BOTH python-pickle and xml-pickle, since
# this is a tricky case to get right, and I want
# to make sure we match the official protocol

def check_foo(o1,o2,NR):
    # .a and .b should match; .c and .d should match the initargs
    if o1.__class__ != Foo or o2.__class__ != Foo or \
           o1.a != o2.a or o1.b != o2.b or \
           o2.c != o1.__getinitargs__()[2] or \
           o2.d != o1.__getinitargs__()[3]:
        raise "ERROR(%d)" % NR
    
x = Foo(1,'abc',[1,2,3],('a','b','c'))

# python-pickle
p = pickle.dumps(x)
x2 = pickle.loads(p)

# test: .a and .b should match x; .c and .d should match
# values from __getinitargs__
check_foo(x, x2, 1)

# xml-pickle
px = xml_pickle.dumps(x)
x3 = xml_pickle.loads(px)

# test: .a and .b should match x; .c and .d should match
# values from __getinitargs__
check_foo(x, x3, 2)

# make sure all call counts are correct
# 1 __init__ per loads(), plus original object = 3
# 1 __getinitargs__ per dumps(), plus 2 extra calls per test = 6
if COUNT_INIT != 3 or COUNT_GETARGS != 6:
    raise "ERROR(3)"

# do a multilevel object to test all three cases
# (toplevel, attr, item)

# toplevel
f = Foo(1,2,3,4)
# first level attr
f.x = Foo(5,6,7,8)
# as item in a UserList
f.y = UserList(['x','y','z'])
f.y += [Foo(9,10,11,12),UserList(['a','b','c'])] 
# list inside list
f.z = [1, 2, 3, UserList([4, 5, Foo(13,14,15,16)])]

x = xml_pickle.dumps(f)
o = xml_pickle.loads(x)

# check all
check_foo( f, o, 4 )
check_foo( f.x, o.x, 5 )
check_foo( f.y[3], o.y[3], 5 )
check_foo( f.z[3][2], o.z[3][2], 6 )

print "** OK **"
