"Demonstrate that rawpickle works (by saving SREs as rawpickles)"

import UserList, UserDict
from types import *
import	re, StringIO
import gnosis.xml.pickle.ext as xml_pickle_ext
import gnosis.xml.pickle as xml_pickle
from gnosis.xml.pickle.util import setParanoia
from gnosis.xml.pickle.ext._mutate import __disable_extensions
from gnosis.xml.pickle.ext import mutate
import funcs

funcs.set_parser()

class foo_class:
    def __init__(self):
        pass

def checkfoo(o1,o2):
    if o1.__class__ != foo_class or \
       o2.__class__ != foo_class:
        raise "ERROR(0)"
    
    for attr in ['sre','d','ud','l','ul','tup']:
        if getattr(o1,attr) != getattr(o2,attr):
            raise "ERROR(1)"
    
def printfoo(obj):
    print type(obj.sre), obj.sre.pattern
    print type(obj.d), ":%s:%s:" % \
          (obj.d['One'].pattern, obj.d['Two'].pattern)
    print type(obj.ud), ":%s:%s:" % \
          (obj.ud['OneOne'].pattern, obj.ud['TwoTwo'].pattern)
    print type(obj.l), ":%s:%s" % \
          (obj.l[0].pattern, obj.l[1].pattern)
    print type(obj.ul), ":%s:%s" % \
          (obj.ul[0].pattern, obj.ul[1].pattern)
    print type(obj.tup), ":%s:%s:" % \
          (obj.tup[0].pattern, obj.tup[1].pattern)

foo = foo_class()

# allow imported classes to be restored
setParanoia(0)

# test both code paths

# path 1 - non-nested ('attr' nodes)
foo.sre = re.compile('\S+\s+\S+sss')

# path 2 - nested ('item/key/val' nodes)
foo.d = { 'One': re.compile('[abc]+<escape me>\n[1-9]*'),
          'Two': re.compile('\s+1234[a-z]+$') }

foo.ud = UserDict.UserDict()
foo.ud['OneOne'] = re.compile('^He[l]+o\s+')
foo.ud['TwoTwo'] = re.compile('world$')

foo.l = []
foo.l.append( re.compile('[foo|bar]?') )
foo.l.append( re.compile('[qrs]+[1-9]?') )

foo.ul = UserList.UserList()
foo.ul.append( re.compile('[+\-][0-9]+') )
foo.ul.append( re.compile('(bored yet)?') )

foo.tup = ( re.compile('this is [not]? '), re.compile('a [tuple|list]') )

#print "---PRE-PICKLE---"
#printfoo(foo)

# turn off extensions so that SREs will be saved as rawpickles
__disable_extensions()

x1 = xml_pickle.dumps(foo)
#print x1

#print "---POST-PICKLE---"
bar = xml_pickle.loads(x1)
#printfoo(bar)

checkfoo(foo,bar)

# same thing on copy
x2 = xml_pickle.dumps(bar)
baz = xml_pickle.loads(x2)

checkfoo(bar,baz)

#print "---XML from original---"
#print x1

#print "---XML from copy---"
#print x2

print "** OK **"
