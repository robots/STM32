"""Introspective functions for Python objects"""

import gnosis.pyconfig

def docstrings():
    if not gnosis.pyconfig.Can_AssignDoc(): return
    """
    containers.__doc__   = "A tuple of built-in types that contain parts"
    simpletypes.__doc__  = "A tuple of built-in types that have no parts"
    datatypes.__doc__	   = "A tuple of all the built-in types that hold data"
    immutabletypes.__doc__="A tuple of built-in types that are immutable"
    """
    isContainer.__doc__  = "this and that"

import sys, string

from types import *
from operator import add
from gnosis.util.combinators import or_, not_, and_, lazy_any

containers = (ListType, TupleType, DictType)
simpletypes = (IntType, LongType, FloatType, ComplexType, StringType)
if gnosis.pyconfig.Have_Unicode():
    simpletypes = simpletypes + (UnicodeType,)
datatypes = simpletypes+containers
immutabletypes = simpletypes+(TupleType,)

class undef: pass

def isinstance_any(o, types):
    "A varargs form of isinstance()"
    for t in types:
        if isinstance(o, t): return t

isContainer	 = lambda o: isinstance_any(o, containers)
isSimpleType = lambda o: isinstance_any(o, simpletypes)
isInstance	 = lambda o: type(o) is InstanceType
isImmutable	 = lambda o: isinstance_any(o, immutabletypes)

if gnosis.pyconfig.Have_ObjectClass():	
    isNewStyleInstance = lambda o: issubclass(o.__class__,object) and \
                                not type(o) in datatypes
else:
    isNewStyleInstance = lambda o: 0
isOldStyleInstance = lambda o: isinstance(o, ClassType)
isClass			= or_(isOldStyleInstance, isNewStyleInstance)

if gnosis.pyconfig.Have_ObjectClass():
    def isNewStyleClass(o):
        try:
            # would give a NameError on Python <= 2.1
            return issubclass(o,object)
        except TypeError:
            return 0
else:
    def isNewStyleClass(o):	
        return 0

hasSlots	 = lambda o: hasattr(o,'__slots__')
hasInit		 = lambda o: hasattr(o,'__init__')
hasDictAttrs = lambda o: (hasattr(o,'__dict__') and o.__dict__)

isInstanceLike = lazy_any(isInstance, hasDictAttrs, hasSlots)
hasCompoundShape = and_(isInstanceLike, not_(isInstance))

true_container = lambda o: type(o) in containers
true_simpletype = lambda o: type(o) in simpletypes
true_datatype = or_(true_container, true_simpletype)
child_container = and_(not_(true_container), isContainer)
child_simpletype = and_(not_(true_simpletype), isSimpleType)
child_datatype = or_(child_container, child_simpletype)

def hasCoreData(o):
    """Is 'o' an object subclassed from a builtin type?

    (i.e. does it contain data other than attributes)
    We only want subclasses, not the class itself (otherwise we'd
    catch ALL lists, integers, etc.)
    """
    return child_datatype(o)

# semantic convenience - maybe it'll do something else later?
wantsCoreData = hasCoreData

def attr_dict(o, fillslots=0):
    if hasattr(o,'__dict__'):
        return o.__dict__
    elif hasattr(o,'__slots__'):
        dct = {}
        for attr in o.__slots__:
            if fillslots and not hasattr(o, attr):
                setattr(o, attr, undef())
                dct[attr] = getattr(o,attr)
            elif hasattr(o, attr):
                dct[attr] = getattr(o,attr)
        return dct
    else:
        raise TypeError, "Object has neither __dict__ nor __slots__"

attr_keys = lambda o: attr_dict(o).keys()
attr_vals = lambda o: attr_dict(o).values()

def attr_update(o,new):
    for k,v in new.items():
        setattr(o,k,v)

def data2attr(o):
    "COPY (not move) data to __coredata__ 'magic' attribute"
    o.__coredata__ = getCoreData(o)
    return o

def attr2data(o):
    "Move 'magic' attribute back to 'core' data"
    if hasattr(o,'__coredata__'):
        o = setCoreData(o, o.__coredata__)
        del o.__coredata__
    return o

def setCoreData(o, data, force=0):
    "Set core data of obj subclassed from a builtin type, return obj"
    #-- Only newobjects unless force'd
    if not force and not wantsCoreData(o): pass
    #-- Tuple and simpletypes are immutable (need new copy)!
    elif isImmutable(o):
        # Python appears to guarantee that all classes subclassed
        # from immutables must take one and only one argument
        # to __init__ [calling __init__() is not usually allowed
        # when unpickling] ... lucky us :-)
        new = o.__class__(data)
        attr_update(new, attr_dict(o))	# __slots__ safe attr_dict()
        o = new
    elif isinstance(o, DictType):
        o.clear()
        o.update(data)
    elif isinstance(o, ListType):
        o[:] = data
    return o

def getCoreData(o):
    "Return the core data of object subclassed from builtin type"
    if hasCoreData(o):
        return isinstance_any(o, datatypes)(o)
    else:
        raise TypeError, "Unhandled type in getCoreData for: ", o

def instance_noinit(C):
    """Create an instance of class C without calling __init__

    [Note: This function was greatly simplified in gnosis-1.0.7,
    but I'll leave these notes here for future reference.]

    We go to some lengths here to avoid calling __init__.  It
    gets complicated because we cannot always do the same thing;
    it depends on whether there are __slots__, and even whether
    __init__ is defined in a class or in its parent.  Even still,
    there are ways to construct *highly artificial* cases where
    the wrong thing happens.

    If you change this, make sure ALL test cases still work ...
    easy to break things
    """
    if isOldStyleInstance(C):
        import new
        # 2nd arg is required for Python 2.0 (optional for 2.1+)
        return new.instance(C,{})
    elif isNewStyleInstance(C):
        return C.__new__(C)
    else:
        raise TypeError, "You must specify a class to create instance of."

if __name__ == '__main__':
    "We could use some could self-tests (see test/ subdir though)"
else:
    docstrings()

