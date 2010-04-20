"Test harness for measuring the speed of different parsers. --fpm"

# On a pure-speed level, the SAX parser is only
# about 3-4x faster than the DOM parser.
#
# However, SAX really shines on larger XML files
# where DOM starts swapping.

# Results, gnosis_utils-1.0.1:
#
# 4-level structure, 1 Mb XML text (20874 elements)
#
# Python  write   minidom   DOM    SAX
#   2.0    5.3      44       82     22
#   2.1    4.6      41       75     24
#   2.2    5.0      35       65     20

# 5-levels, 7.7 Mb XML text (146113 elements)
#
# Python  write   minidom   DOM    SAX
#   2.0    38        x       x     148
#   2.1    32        x       x     135
#   2.2    35        x       x     134

# 6-levels, 50 Mb XML text (897548 elements)
#
# Python  write   minidom   DOM    SAX
#   2.0    234       x       x     908
#   2.1    211       x       x     836
#   2.2    222       x       x     849

# "x" denotes where DOM would run for hours, so total time
# was not measured.

import gnosis.xml.pickle as xml_pickle
from xml.dom import minidom
from UserList import UserList
from UserDict import UserDict
import re, os
from stat import *
from time import time

class foo: pass
def mk_foo(level=5):
    f = foo()
    # 1st level obj
    l = [1,2,3,4,5,6,7,8,9,10]
    t = (1.2,2.3,3.4,4.5,5.6,6.7,7.8,8.9,9.1,10.2)
    d = {'one':1,'two':2,'three':3,'four':4,'five':5,'six':6,'seven':7,
         'eight':8,'nine':9,'ten':10}
    c = [(1+2j),(2+3j),(3+4j),(4+5j),(5+6j),(6+7j),(7+8j),(8+9j),(9+10j),(10+11j)]
    s = ('one','two','three','four','five','six','seven','eight','nine','ten')
    # 2nd level obj
    ll = [l,l,l,l,l,
          t,t,t,t,t,
          d,d,d,d,d,
          c,c,c,c,c,
          s,s,s,s,s]
    # 3rd level obj
    u = [ll,ll,ll,ll,ll,ll,ll,ll]
    # 4th level
    f.u = [u,u,u,u,u,u,u,u]
    # 5th level
    if level >= 5: f.uu = [f.u,f.u,f.u,f.u,f.u,f.u]
    # 6th level
    if level >= 6: f.uuu = [f.uu,f.uu,f.uu,f.uu,f.uu,f.uu]
    return f

def doit3():
    class foo: pass
    f = foo()
    f.a = 1
    f.b = f.a
    f.c = "abc"
    f.d = f.c
    f.r = re.compile('aaa\s+[0-9]*')
    f.r2 = f.r
    l = [1,2,3]
    f.u = UserList([l,l])
    x = xml_pickle.dumps(f)
    print x
    g = thing_from_sax2(None,x)
    print f.u, f.r.pattern, f.r2.pattern

def doit2():
    u = UserList([1,2,[(3,4,5),(6,7,8)],3,UserList([0,1,2]),4])
    x = xml_pickle.dumps(u)
    print x
    g = thing_from_sax2(None,x)
    print g

def doit(deepcopy=1):
    f = mk_foo()
    xml_pickle.setDeepCopy(deepcopy)

    print "CREATE XML"
    t1 = time()
    fh = open('aaa.xml','w')
    x = xml_pickle.dump(f,fh)
    fh.close()
    print "TIME = %f"%(time()-t1)
    print "Pickle len = ",os.stat('aaa.xml')[ST_SIZE]

    print "minidom pure parse"
    t1 = time()
    fh = open('aaa.xml','r')
    #minidom.parse(fh)
    fh.close()
    print "TIME = %f"%(time()-t1)

    print "DOM load"
    t1 = time()
    fh = open('aaa.xml','r')
    xml_pickle.setParser("DOM") # default, but just to be sure
    #o = xml_pickle.load(fh)
    fh.close()
    print "TIME = %f"%(time()-t1)
    #del o

    print "SAX load"
    t1 = time()
    fh = open('aaa.xml','r')
    xml_pickle.setParser("SAX")
    m = xml_pickle.load(fh)
    fh.close()
    print "TIME = %f"%(time()-t1)
    del m

    print "cEXPAT load"
    t1 = time()
    fh = open('aaa.xml','r')
    xml_pickle.setParser("cEXPAT")
    #m = xml_pickle.load(fh)
    fh.close()
    print "TIME = %f"%(time()-t1)
    #del m

def pyxml_marshal():
    try:
        from xml.marshal import generic
    except ImportError:
        # Skip comparison with PyXML's xml.marshal if unavailable
        print "Skipping PyXML xml.marshal"
        return

    f = mk_foo()
    print "CREATE XML (xml.marshal style)"
    t1 = time()
    fh = open('bbb.xml','w')
    x = generic.dump(f,fh)
    fh.close()
    print "TIME = %f"%(time()-t1)
    print "Pickle len = ",os.stat('bbb.xml')[ST_SIZE]

    print "xml.marshal load"
    t1 = time()
    fh = open('bbb.xml','r')
    m = generic.load(fh)
    fh.close()
    print "TIME = %f"%(time()-t1)
    del m

pyxml_marshal()
doit(2)

"""
CREATE XML (xml.marshal style)
TIME = 0.050000
Pickle len =  3794
xml.marshal load
TIME = 1.230000
"""


