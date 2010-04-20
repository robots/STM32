"Perform black magic of unearthly and ungodly sorts"

import inspect

from dtdgenerator import *
from metapickler import *

class import_with_metaclass_C(object):
    "This is a class factory, not a metaclass"
    def __init__(self, module, metaclass):
        class Meta(object): __metaclass__ = metaclass
        dct = {'__module__':module}
        mod = __import__(module)
        for key, val in mod.__dict__.items():
            if not inspect.isclass(val):
                setattr(self, key, val)
            else:
                setattr(self, key, type(key,(val,Meta),dct))

def import_with_metaclass(modname, metaklass):
    "Module importer substituting custom metaclass"
    class Meta(object): __metaclass__ = metaklass
    dct = {'__module__':modname}
    mod = __import__(modname)
    for key, val in mod.__dict__.items():
        if inspect.isclass(val):
            setattr(mod, key, type(key,(val,Meta),dct))
    return mod

def from_import(module, names="*"):
    if names == "*":
        names = filter(lambda s: s[0]!="_",dir(module))
    elif names == "**":
        semisafe = lambda s: s not in ('__file__','__name__')
        names = filter(semisafe, dir(module))
    for name in names:
        inspect.currentframe(1).f_globals[name] = getattr(module, name)

