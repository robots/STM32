from gnosis.util.introspect import hasSlots, hasInit
from types import *

class Old_noinit: pass

class Old_init:
    def __init__(self): print "Init in Old"

class New_slots_and_init(int):
    __slots__ = ('this','that')
    def __init__(self): print "Init in New w/ slots"

class New_init_no_slots(int):
    def __init__(self): print "Init in New w/o slots"

class New_slots_no_init(int):
    __slots__ = ('this','that')

class New_no_slots_no_init(int):
    pass

from UserDict import UserDict
class MyDict(UserDict):
    pass

def one():
    def instance_noinit(C):
        "Create an instance of class C without calling __init__"
        if hasSlots(C):
            try:
                # the class defined init - remove it temporarily
                _init = C.__init__
                del C.__init__
                obj = C()
                C.__init__ = _init
            except AttributeError:
                # class did not define its own init, no need to del it
                obj = C()
        else:
            class _emptyclass(C):
                def __init__(self): pass
            obj = _emptyclass()
            obj.__class__ = C
        return obj

    print "----- This should be the only line -----"
    instance_noinit(MyDict)
    instance_noinit(Old_noinit)
    instance_noinit(Old_init)
    instance_noinit(New_slots_and_init)
    instance_noinit(New_slots_no_init)
    instance_noinit(New_init_no_slots)
    instance_noinit(New_no_slots_no_init)

def two():
    def instance_noinit(C):
      """Create an instance of class C without calling __init__"""
      try:
        # this works for "classic" Python instances
        class _emptyclass(C):
            def __init__(self): pass
        obj = _emptyclass()
        obj.__class__ = C
      except:
        # this is for objects with __slots__
        if hasattr(C,'__init__') and isinstance(C.__init__,MethodType):
            # the class defined init - remove it temporarily
            _init = C.__init__
            del C.__init__
            obj = C()
            C.__init__ = _init
        else:
            # class did not define init, and we can't del the
            # builtin init for slot support
            obj = C()
      return obj

    print "----- Same test, fpm version of instance_noinit() -----"
    instance_noinit(MyDict)
    instance_noinit(Old_noinit)
    instance_noinit(Old_init)
    instance_noinit(New_slots_and_init)
    instance_noinit(New_slots_no_init)
    instance_noinit(New_init_no_slots)
    instance_noinit(New_no_slots_no_init)

def three():
    def instance_noinit(C):
        "Create an instance of class C without calling __init__"
        if hasattr(C,'__init__') and isinstance(C.__init__,MethodType):
            # the class defined init - remove it temporarily
            _init = C.__init__
            print _init
            del C.__init__
            obj = C()
            C.__init__ = _init
        else:
            # class did not define init, no need to delete it
            obj = C()
        return obj

    print "----- Same test, dqm version of instance_noinit() -----"
    instance_noinit(MyDict)
    instance_noinit(Old_noinit)
    instance_noinit(Old_init)
    instance_noinit(New_slots_and_init)
    instance_noinit(New_slots_no_init)
    instance_noinit(New_init_no_slots)
    instance_noinit(New_no_slots_no_init)

one()
two()
three()
