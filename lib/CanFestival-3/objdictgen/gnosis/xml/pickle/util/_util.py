import gnosis.xml.pickle
from types import *
import sys
CLASS_STORE = {}

class _EmptyClass: pass

# get my modulename (on-the-fly classes live here)
dynamic_module = _EmptyClass().__class__.__module__

def dbg(s):
    #print s
    pass

# adapted from pickle.py, whichmodule()
def get_function_info( func ):

    fname = func.__name__

    for name, module in sys.modules.items():
        if name != '__main__' and \
            hasattr(module, fname) and \
            getattr(module, fname) is func:
            break
    else:
        name = '__main__'

    return (name,func.__name__)

def unpickle_function( module,name,paranoia ):
    "Load a function, given module.name as returned by get_function_info()"
    if paranoia > 0:
        return None  # not allowed for paranoia > 0

    # module must already be imported for paranoia == 0
    if paranoia < 0:
        __import__(module)

    mod = sys.modules.get(module,None)
    if mod is None:
        return None

    if hasattr(mod,name):
        return getattr(mod,name)
    return None

#--- Helper functions for creating objects by name ---
# (these are pretty cool to have, even without xml_pickle)

def _get_class_from_locals(dict, modname, classname):
    for name in dict.keys():
        # class imported by caller
        if name == modname and type(dict[name]) is ModuleType:
            mod = dict[name]
            if hasattr(mod,classname):
                return getattr(mod,classname)
        # class defined by caller
        if name == classname and dict[name].__module__ == modname:
            return dict[name]
    return None

def get_class_from_sysmodules(modname, classname):
    "Get a class by looking in sys.modules"
    # check global modules
    try:
        return getattr(sys.modules[modname],classname)
    except (KeyError, AttributeError):
        return None

def get_class_from_stack(modname, classname):
    "Get a class ONLY IF already been imported or created by caller"
    stk = _mini_getstack()
    for frame in stk:
        k = _get_class_from_locals(frame.f_locals,modname,classname)
        if k: return k

def get_class_full_search(modname, classname):
    "Get a class, importing if necessary"
    __import__(modname)
    mod = sys.modules[modname]
    if hasattr(mod,classname):
        return getattr(mod,classname)
    return None

def get_class_from_store(classname):
    "Get the class from the store, if possible"
    return CLASS_STORE.get(classname, None) or \
           gnosis.xml.pickle.__dict__.get(classname,None)

def add_class_to_store(classname='', klass=None):
    "Put the class in the store (as 'classname'), return CLASS_STORE"
    if classname and klass:
        CLASS_STORE[classname] = klass
    return CLASS_STORE

def remove_class_from_store(classname):
    "Remove the classname from the store, return CLASS_STORE"
    try: del CLASS_STORE[classname]
    except: pass
    return CLASS_STORE

def get_class_from_vapor(classname):
    " Create new class from nothing"
    exec('class %s: pass' % classname)
    k = locals()[classname]
    return k

# -- functions for dynamic object creation --

from gnosis.util.introspect import instance_noinit

def obj_from_classtype(klass):
    """Create an object of ClassType klass. We aren't
    allowed (by the pickle protocol) to call __init__() on
    the new object, so we have to be careful."""

    return instance_noinit(klass)

def get_class_from_name(classname, modname=None, paranoia=1):
    """Given a classname, optional module name, return a ClassType,
    of type module.classname, obeying the PARANOIA rules."""

    if paranoia <= 2:                # first, try our store
        klass = get_class_from_store(classname)
        if klass:
            dbg("**GOT CLASS FROM STORE**")
            return klass

    # get_class_from_stack() is more paranoid, but unless we
    # define a new paranoid level for it, there's no need
    # to try it if the sys.modules method works. this is
    # faster so try it first.
    if paranoia <= 0 and modname:    # try sys.modules
        klass = get_class_from_sysmodules(modname, classname)
        if klass:
            dbg("**GOT CLASS FROM sys.modules**")
            return klass

    # xxx move up one level if we define a new paranoia level for it
    if paranoia <= 0 and modname:    # next, try our caller's namespace
        klass = get_class_from_stack(modname, classname)
        if klass:
            dbg("**GOT CLASS FROM STACK**")
            return klass

    if paranoia <= -1 and modname:   # next, try importing the module
        klass = get_class_full_search(modname, classname)
        if klass:
            dbg("**GOT CLASS FROM CALLER**")
            return klass

    # careful -- needs to be a fallback if any of the above fail
    if paranoia <= 1:                # finally, create from thin air
        klass = get_class_from_vapor(classname)
        dbg("**GOT CLASS FROM VAPOR**")
        return klass

    dbg("**ERROR - couldn't get class - paranoia = %s" % str(paranoia))

    # *should* only be for paranoia == 2, but a good failsafe anyways ...
    raise XMLUnpicklingError, \
          "Cannot create class under current PARANOIA setting!"

def obj_from_name(classname, modname=None, paranoia=1):
    """Given a classname, optional module name, return an object
    of type module.classname, obeying the PARANOIA rules.
    Does NOT call __init__ on the new object, since the caller
    may need to do some other preparation first."""

    dbg("OBJ FROM NAME %s.%s" % (str(modname),str(classname)))
    klass = get_class_from_name(classname,modname,paranoia)
    dbg("KLASS %s" % str(klass))
    return obj_from_classtype(klass)

# -- get class/module names from an object --

def _klass(thing):
    return thing.__class__.__name__

def _module(thing):
    """If thing's class is located in CLASS_STORE, was created
    from "thin-air", or lives in the "xml_pickle" namespace,
    don't write the module name to the XML stream (without these
    checks, the XML is functional, but ugly -- module names like
    "gnosis.xml.pickle.util._util")
    """
    klass = thing.__class__
    if klass.__module__ == dynamic_module: return None
    if klass in CLASS_STORE.values(): return None
    if klass in gnosis.xml.pickle.__dict__.values(): return None
    return thing.__class__.__module__

def safe_eval(s):
    if 0:   # Condition for malicious string in eval() block
        raise "SecurityError", \
              "Malicious string '%s' should not be eval()'d" % s
    else:
        return eval(s)

def safe_string(s):
    if isinstance(s, UnicodeType):
        raise TypeError, "Unicode strings may not be stored in XML attributes"

    # markup XML entities
    s = s.replace('&', '&amp;')
    s = s.replace('<', '&lt;')
    s = s.replace('>', '&gt;')
    s = s.replace('"', '&quot;')
    s = s.replace("'", '&apos;')
    # for others, use Python style escapes
    s = repr(s)
    return s[1:-1]  # without the extra single-quotes

def unsafe_string(s):
    # for Python escapes, exec the string
    # (niggle w/ literalizing apostrophe)
    s = s.replace("'", r"\047")
    exec "s='"+s+"'"
    # XML entities (DOM does it for us)
    return s

def safe_content(s):
    "Markup XML entities and strings so they're XML & unicode-safe"
    s = s.replace('&', '&amp;')
    s = s.replace('<', '&lt;')
    s = s.replace('>', '&gt;')

    # wrap "regular" python strings as unicode
    if isinstance(s, StringType):
        s = u"\xbb\xbb%s\xab\xab" % s

    return s.encode('utf-8')

def unsafe_content(s):
    """Take the string returned by safe_content() and recreate the
    original string."""
    # don't have to "unescape" XML entities (parser does it for us)

    # unwrap python strings from unicode wrapper
    if s[:2]==unichr(187)*2 and s[-2:]==unichr(171)*2:
        s = s[2:-2].encode('us-ascii')

    return s

def subnodes(node):
    # for PyXML < 0.8, childNodes are all <DOM Elements>, so we
    # just remove the #text nodes.
    # for PyXML > 0.8, childNodes includes both <DOM Elements> and
    # DocumentType objects, so we have to separate them.
    return filter(lambda n: hasattr(n,'_attrs') and \
                  n.nodeName<>'#text', node.childNodes)

#-------------------------------------------------------------------
# Python 2.0 doesn't have the inspect module, so we provide
# a "mini" implementation (also faster since it doesn't lookup
# unneeded info).
#
# This was adapted from the real inspect module, written by
# Ka-Ping Yee <ping@lfw.org>.
#--------------------------------------------------------------------
def _mini_getstack():
    frame = _mini_currentframe().f_back
    framelist = []
    while frame:
        framelist.append(frame)
        frame = frame.f_back
    return framelist

def _mini_currentframe():
    try:
        raise 'catch me'
    except:
        return sys.exc_traceback.tb_frame.f_back
# --- End of "mini" inspect module ---

