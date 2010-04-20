
from types import *
from gnosis.util.introspect import isInstanceLike, hasCoreData
import gnosis.pyconfig

XMLPicklingError = "gnosis.xml.pickle.XMLPicklingError"
XMLUnpicklingError = "gnosis.xml.pickle.XMLUnpicklingError"

# hooks for adding mutators
# each dict entry is a list of chained mutators
_mutators_by_classtype = {}
_unmutators_by_tag = {}

# for test purposes ... (hm ... could it be useful for some reason?)
def __disable_extensions():
    global _mutators_by_classtype
    _mutators_by_classtype = {}
    for tag in _unmutators_by_tag.keys():
        # rawpickle is always on -- we have to fallback on
        # it when everything else fails
        if tag != "rawpickle":
            del _unmutators_by_tag[tag]

_has_coredata_cache = {}

# sanity in case Python changes ...
if gnosis.pyconfig.Have_BoolClass() and gnosis.pyconfig.IsLegal_BaseClass('bool'):
    raise XMLPicklingError, \
        "Assumption broken - can now use bool as baseclass!"

Have_BoolClass = gnosis.pyconfig.Have_BoolClass()

def get_mutator(obj):
    # note we can't cache the search results since mutators can decide
    # dynamically (based on content) whether to accept objects
    mlist = _mutators_by_classtype.get(type(obj)) or []
    for mutator in mlist:
        if mutator.wants_obj(obj):
            return mutator

    # check for objects derived from the new builtins-as-classes (python 2.2)

    # hasCoreData() is pretty slow (unavoidable, though), so we cache
    # the results. All coredata types go to the same mutator, so this is allowed,
    # unlike above.

    # Bools (in Python 2.3+) are a special case - they look like a newstyle
    # class, yet cannot serve as a base class (they can only be True or
    # False). Therefore, there will never be a need for a mutator, and
    # they can be completely handled in the main "if" block in _pickle.py.
    if Have_BoolClass and type(obj) is BooleanType:
         return None		
    
    if not hasattr(obj,'__class__'):
        return None

    if _has_coredata_cache.has_key(obj.__class__):
        return _has_coredata_cache[obj.__class__]

    if hasCoreData(obj):
        _has_coredata_cache[obj.__class__] = get_unmutator('__compound__',None)
        return get_unmutator('__compound__',None)
    else:
        _has_coredata_cache[obj.__class__] = None
    
    return None

def can_mutate(obj):
    if get_mutator(obj):
        return 1
    else:
        return 0

def mutate(obj):
    mutator = get_mutator(obj)
    tobj = mutator.mutate(obj)

    if not isinstance(tobj,XMLP_Mutated):
        raise XMLPicklingError, \
              "Bad type returned from mutator %s" % mutator
    
    return (mutator.tag,tobj.obj,mutator.in_body,tobj.extra)

# one-step replacement for:
#      if can_mutate():
#          mutate()
# (one get_mutator() call vs. two)

def try_mutate(obj,alt_tag,alt_in_body,alt_extra):
    
    mutator = get_mutator(obj)

    if mutator is None:
        return (alt_tag,obj,alt_in_body,alt_extra)

    tobj = mutator.mutate(obj)

    if not isinstance(tobj,XMLP_Mutated):
        raise XMLPicklingError, \
              "Bad type returned from mutator %s" % mutator
    
    return (mutator.tag,tobj.obj,mutator.in_body,tobj.extra)

def get_unmutator(tag, obj):
    list = _unmutators_by_tag.get(tag) or []
    for unmutator in list:
        if unmutator.wants_mutated(obj):
            return unmutator

    return None

def can_unmutate(tag, obj):
    if get_unmutator(tag,obj):
        return 1
    else:
        return 0

def unmutate(tag, obj, paranoia, mextra):
    unmutator = get_unmutator(tag, obj)

    return unmutator.unmutate(XMLP_Mutated(obj,mextra))

def add_mutator(xmlp_mutator):
    "Register an XMLP_Mutator object"
    # later callers are inserted before earlier callers, which is
    # right in a "late binding" sense

    # some mutators handle multiple types and don't want to be added
    # the the classtype chain. They "hide" by setting class_type=None (not type(None)!!)
    if xmlp_mutator.class_type:
        try:
            _mutators_by_classtype[xmlp_mutator.class_type].insert(0,xmlp_mutator)
        except:
            _mutators_by_classtype[xmlp_mutator.class_type] = [xmlp_mutator]
    try:
        _unmutators_by_tag[xmlp_mutator.tag].insert(0,xmlp_mutator)
    except:
        _unmutators_by_tag[xmlp_mutator.tag] = [xmlp_mutator]

def remove_mutator(xmlp_mutator):
    "De-register an XMLP_Mutator object"
    list = _unmutators_by_tag[xmlp_mutator.tag]
    list.remove(xmlp_mutator)
    list = _mutators_by_classtype[xmlp_mutator.class_type]
    list.remove(xmlp_mutator)

class XMLP_Mutated:
    """This is the type that XMLP_Mutator.mutate() returns.
    Having this as a distinct type will make it easy to add flags,
    etc., in the future without breaking existing mutators.
    In most cases, you just wrap your mutated obj like this:
        return XMLP_Mutated( obj )"""
    def __init__(self,obj,extra=None):
        self.obj = obj
        self.extra = extra
        
class XMLP_Mutator:
    "Parent class for XMLP Mutators"
    # by default, only disable mutators when PARANOIA = 2
    def __init__(self, class_type, tag, paranoia=1, in_body=0):
        """
        class_type = type() that this mutator handles
        tag = Symbolic tag for what this mutator produces
        paranoia = Maximum PARANOIA level at which to enable this
                   mutator (note that PARANOIA for mutators is a
                   static concept - it doesn't matter which
                   namespace we're in, etc. Each mutator is instead
                   judged to be "safe" at a given level based on
                   what the datatype can do).
        in_body = (Applicable only for types that mutate to a string or
                   numeric type.)
                  If in_body == 0, pickled text is placed in the value= attr.
                  If in_body == 1, pickled text is placed in the element body.
        """
        self.class_type = class_type
        self.tag = tag
        self.paranoia = paranoia
        self.in_body = in_body

    def wants_obj(self,obj):
        # by default, we want everything of class_type.
        # derived classes can override if they need specialization.
        # anything not wanted is passed down the line.
        return 1

    def wants_mutated(self,mobj):
        "obj is of type XMLP_Mutated"
        # can I unmutate the mutated object?
        return 1

    def mutate(self,obj):
        """given obj, return an XMLP_Mutated object, where the
        XMLP_Mutated.obj member is a basic type (string,numeric,
        assoc,seq,or PyObject)"""

        # this member is required
        raise NotImplementedError

    def unmutate(self,mobj):
        "take an XMLP_Mutated obj and recreate the original object"

        # this member is required
        raise NotImplementedError
