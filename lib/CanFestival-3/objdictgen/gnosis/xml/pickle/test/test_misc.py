
"""

Here are collected miscellaneous tests (from bug reports,etc.)
that don't clearly fit elsewhere. --fpm

"""

import gnosis.xml.pickle as xml_pickle	
from funcs import set_parser
import gnosis.pyconfig as pyconfig

set_parser()

# this is from a bug report from Even Westvang <even@bengler.no>.
# the problem is that new-style classes used as attributes weren't
# being pickled correctly.

# let it load my classes
xml_pickle.setParanoia(0)

if pyconfig.Have_ObjectClass(): # need new-style classes
    class PickleMeOld:
        def __init__(self): pass

    class PickleMeNew(object):
        def __init__(self): pass

    class Container(object):
        def __init__(self, klass):
            self.classRef = klass

    x = xml_pickle.dumps(Container(PickleMeOld))
    o = xml_pickle.loads(x)
    if o.classRef != PickleMeOld:
        raise "ERROR(1)"
    
    x = xml_pickle.dumps(Container(PickleMeNew))
    o = xml_pickle.loads(x)
    if o.classRef != PickleMeNew:
        raise "ERROR(2)"

print "** OK **"
    
