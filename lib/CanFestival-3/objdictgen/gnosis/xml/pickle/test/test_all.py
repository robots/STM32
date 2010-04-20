"""
generic test harness - runs "all" the test_*.py files and
writes the output to TESTS.OUT-{pyversion}

-- frankm@hiwaay.net
"""

import os,sys,string
from time import time
from gnosis.xml.pickle.util import enumParsers
import funcs
import gnosis.pyconfig
from funcs import unlink, touch

# use same python that we're running under
py = sys.executable

# these are ordered (roughly) from simplest->hardest, in terms
# of sophistication of the parser -- ie. when building a new
# parser, you might want to build up capability in this order

# NOTE! Not *ALL* test_*.py files are suitable for a batch run,
# so we list the tests we want
maintests = """
    bltin
    basic
    mixin
    compat
    ftypes
    getstate
    getinitargs
    init
    modnames
    paranoia
    mutators
    rawp_sre
    re
    ref
    selfref
    unicode
    bools_ro
    misc
"""

import string # py1.5 compat

def echof(filename,line):
    if os.path.isfile(filename):
        f = open(filename,'a')
    else:
        f = open(filename,'w')

    f.write(line+'\n')

def pechof(filename,line):
    print line
    if os.path.isfile(filename):
        f = open(filename,'a')
    else:
        f = open(filename,'w')

    f.write(line+'\n')

#tout = 'TESTS.OUT-%s' % os.path.split(py)[-1]
tout = 'TESTS.OUT-%s-%s' % (sys.platform,sys.version.split()[0])

unlink(tout)
touch(tout)

tests = []
for name in string.split(maintests):
    tests.append( 'test_%s.py' % name )

if gnosis.pyconfig.Have_Generators() and \
   gnosis.pyconfig.Have_Module('itertools'):
    tests.append('test_objectify.py')
else:
    pechof(tout,"*** OMITTING test_objectify.py")

if gnosis.pyconfig.IsLegal_BaseClass('unicode'):
    tests.append('test_badstring.py')
else:
    pechof(tout,"*** OMITTING test_badstring.py")

if gnosis.pyconfig.Have_BoolClass():
    tests.append('test_bools.py')
else:
    pechof(tout,"** OMITTING test_bools.py")
    
if gnosis.pyconfig.Have_ObjectClass() and gnosis.pyconfig.Have_Slots():
    # tests requiring Python >= 2.2
    tests.append('test_subbltin.py')
    tests.append('test_slots.py')
else:
    pechof(tout,"** OMITTING test_subbltin.py")
    pechof(tout,"** OMITTING test_slots.py")

try:
    import gzip
    tests.append('test_4list.py')
except:
    pechof(tout,"** OMITTING test_4list (missing zlib) **")

# if mx.DateTime installed, add those tests
try:
    import mx.DateTime
    tests.append('test_mx.py')
    tests.append('test_rawp_mx.py')
except:
    pechof(tout,"** OMITTING test_mx.py & test_rawp_mx.py")

# if Random works, add setstate test
try:
    import random
    r = random.Random()
    #---begin bug check---
    # XXX as of Python 2.3b1, pickle of Randoms is broken.
    # remove this test as soon as it is fixed (note that
    # test_getstate also tests __setstate__, so I think
    # we still have complete coverage, but this test
    # is nice too since it's somewhat different)
    import pickle
    pickle.dumps(r)
    #---end bug check---
    
    tests.append('test_setstate.py')
except:
    pechof(tout,"** OMITTING test_setstate.py")

# if Numeric installed, add numpy test
try:
    import Numeric
    tests.append('test_numpy.py')
except:
    pechof(tout,"** OMITTING test_numpy.py")

# sanity check the test harness before starting
def check_harness():
    # try to redirect stderr to a file so that the intentional
    # exceptions in the "fail" tests below won't freak out the user
    if os.name == 'posix':
        outstr = '2>&1 > harness_check.out'
    elif os.name == 'nt':
        outstr = '2>1> harness_check.out'
    else:
        # don't know how to redirect stderr here ...
        outstr = '> harness_check.out'
        
    # known "pass" tests
    for good in ['test_pass_1.py','test_pass_2.py','test_pass_3.py']:
        r = os.system('%s %s %s' % (py,good,outstr))
        if r != 0:
            pechof(tout,"****** Harness test failed ******")
            sys.exit(1)

    # known "fail" tests
    print "***************** INGORE EXCEPTIONS BETWEEN THESE LINES *****************"	
    for bad in ['test_fail_exit.py','test_fail_raise_1.py',
                'test_fail_raise_2.py','test_fail_raise_3.py']:
        r = os.system('%s %s %s' % (py,bad,outstr))
        if r == 0:
            pechof(tout,"****** Harness test failed ******")
            sys.exit(1)

    print "***************** INGORE EXCEPTIONS BETWEEN THESE LINES *****************"
    
    unlink('harness_check.out')

import gnosis.version
pechof(tout,"*** Running all xml.pickle tests, Gnosis Utils %s" % \
       gnosis.version.VSTRING)

# check test harness
check_harness()
pechof(tout,"Sanity check: OK")
      
# get available parsers
parser_dict = enumParsers()

# test with DOM parser, if available
if parser_dict.has_key('DOM'):

    # make sure the USE_.. files are gone
    unlink("USE_SAX")
    unlink("USE_CEXPAT")

    t1 = time()

    for test in tests:
        print "Running %s" % test
        echof(tout,"** %s %s DOM PARSER **" % (py,test))
        r = os.system("%s %s >> %s"%(py,test,tout))
        if r != 0:
            pechof(tout,"***ERROR***")
            sys.exit(1)

    pechof(tout,"%.1f seconds" % (time()-t1))
else:
    pechof(tout,"** SKIPPING DOM parser **")

# test with SAX parser, if available
if parser_dict.has_key("SAX"):

    touch("USE_SAX")

    t1 = time()

    for test in tests:
        print "Running %s" % test
        echof(tout,"** %s %s SAX PARSER **" % (py,test))
        r = os.system("%s %s >> %s"%(py,test,tout))
        if r != 0:
            pechof(tout,"***ERROR***")
            sys.exit(1)

    pechof(tout,"%.1f seconds" % (time()-t1))

    unlink("USE_SAX");
else:
    pechof(tout,"** SKIPPING SAX parser **")

# test with cEXPAT parser, if available
if parser_dict.has_key("cEXPAT"):

    touch("USE_CEXPAT");

    t1 = time()

    for test in tests:
        print "Running %s" % test
        echof(tout,"** %s %s CEXPAT PARSER **" % (py,test))
        r = os.system("%s %s >> %s"%(py,test,tout))
        if r != 0:
            pechof(tout,"***ERROR***")
            sys.exit(1)

    pechof(tout,"%.1f seconds" % (time()-t1))

    unlink("USE_CEXPAT");
else:
    pechof(tout,"** SKIPPING cEXPAT parser **")

pechof(tout,"***** ALL TESTS COMPLETED *****")
