
import gnosis.util.introspect as insp
import sys
from funcs import pyver

def test_list( ovlist, tname, test ):

    for o,v in ovlist:
        sys.stdout.write('OBJ %s ' % str(o))

        if (v and test(o)) or (not v and not test(o)):
            print "%s = %d .. OK" % (tname,v)
        else:
            raise "ERROR - Wrong answer to test."
        
# isContainer
ol = [ ([], 1),
       ((1,2), 1),
       ({},1),
       (1,0) ]

test_list( ol, 'isContainer', insp.isContainer )

class foo1:
    pass

class foo2(foo1):
    pass

if pyver() >= '2.2':
    class foo3(object):
        pass
else:
    # make a dummy foo3 to suffice as 'not an instance/class' below
    def foo3():
        return 1
    
# isInstance
ol = [ (foo1(), 1),
       (foo2(), 1),
       (foo3(), 0) ]

test_list( ol, 'isInstance', insp.isInstance)

# isInstanceLike
ol = [ (foo1(), 1),
       (foo2(), 1),
       (foo3(), 0)]

test_list( ol, 'isInstanceLike', insp.isInstanceLike)

from types import *

def is_oldclass(o):
    if isinstance(o,ClassType):
        return 1
    else:
        return 0

ol = [ (foo1,1),
       (foo2,1),
       (foo3,0)]

test_list(ol,'is_oldclass',is_oldclass)

if pyver() >= '2.2':
    # isNewStyleClass
    ol = [ (foo1,0),
           (foo2,0),
           (foo3,1),
           (foo3(),0)]

    test_list(ol,'isNewStyleClass',insp.isNewStyleClass)

# isImmutable
ol = [ ('abc',1),
       (u'abc',1),
       (1,1),
       (1.2,1),
       ((1,2),1),
       ([],0)]
       
test_list(ol, 'isImmutable', insp.isImmutable)

    



    

