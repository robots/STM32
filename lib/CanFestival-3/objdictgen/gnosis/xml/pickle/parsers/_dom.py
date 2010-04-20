from gnosis.xml.pickle.util import subnodes, _EmptyClass, unsafe_string, \
     unsafe_content, safe_eval, obj_from_name, unpickle_function, \
     get_class_from_name
from gnosis.util.introspect import attr_update
from types import *
import gnosis.xml.pickle.ext as mutate
from xml.dom import minidom
from gnosis.util.XtoY import to_number
import gnosis.pyconfig as pyconfig

# Get appropriate array type.
try:
    from Numeric import *
    array_type = 'NumPy_array'
except ImportError:
    from array import *
    array_type = 'array'

# Define exceptions and flags
XMLPicklingError = "gnosis.xml.pickle.XMLPicklingError"
XMLUnpicklingError = "gnosis.xml.pickle.XMLUnpicklingError"

# Define our own TRUE/FALSE syms, based on Python version.
if pyconfig.Have_TrueFalse():
    # Python 2.2 and up have a True/False (even though it's
    # a completely different value between 2.2 & 2.3)
    TRUE_VALUE = True
    FALSE_VALUE = False
else:
    # Below 2.2 has no True/False, so define them as they
    # are in 2.2 (this allows Python < 2.2 to read pickles
    # with bools created by Python 2.2+. Of course, if those
    # pickles are then rewritten, they'll lose their true/false
    # meaning, but hey, there's only so much we can do! :-)
    TRUE_VALUE = 1
    FALSE_VALUE = 0

# entry point expected by XML_Pickle
def thing_from_dom(fh, paranoia=1):
    global visited
    visited = {}
    return _thing_from_dom(minidom.parse(fh),None,paranoia)

def _save_obj_with_id(node, obj):
    id = node.getAttribute('id')
        
    if len(id):		# might be None, or empty - shouldn't use as key
        visited[id] = obj

def unpickle_instance(node, paranoia):
    """Take a <PyObject> or <.. type="PyObject"> DOM node and unpickle the object."""

    # we must first create an empty obj of the correct	type and place
    # it in visited{} (so we can handle self-refs within the object)
    pyobj = obj_from_node(node, paranoia)
    _save_obj_with_id(node, pyobj)

    # slurp raw thing into a an empty object
    raw = _thing_from_dom(node, _EmptyClass(), paranoia)

    # code below has same ordering as pickle.py

    # pass initargs, if defined
    try:
        args = raw.__getinitargs__
        delattr(raw,'__getinitargs__') # don't want this in pyobj (below)
        apply(pyobj.__init__,args)
    except:
        pass

    # next, decide what "stuff" is supposed to go into pyobj
    if hasattr(raw,'__getstate__'):
        stuff = raw.__getstate__
    else:
        stuff = raw.__dict__

    # finally, decide how to get the stuff into pyobj
    if hasattr(pyobj,'__setstate__'):
        pyobj.__setstate__(stuff)
    else:
        if type(stuff) is DictType:	 # must be a Dict if no __setstate__
            # see note in pickle.py/load_build() about restricted
            # execution -- do the same thing here
            #try:
            #	pyobj.__dict__.update(stuff)
            #except RuntimeError:
            #	for k,v in stuff.items():
            #		setattr(pyobj, k, v)
            attr_update(pyobj, stuff)
        else:
            # subtle -- this can happen either because the class really
            # does violate the pickle protocol, or because PARANOIA was
            # set too high, and we couldn't create the real class, so
            # __setstate__ is missing (and __stateinfo__ isn't a dict)
            raise XMLUnpicklingError, \
                  "Non-DictType without setstate violates pickle protocol."+\
                  "(PARANOIA setting may be too high)"

    return pyobj

def obj_from_node(node, paranoia=1):
    """Given a <PyObject> node, return an object of that type.
    __init__ is NOT called on the new object, since the caller may want
    to do some additional work first.
    """
    classname = node.getAttribute('class')
    # allow <PyObject> nodes w/out module name
    # (possibly handwritten XML, XML containing "from-air" classes,
    # or classes placed in the CLASS_STORE)
    try:
        modname = node.getAttribute('module')
    except:
        modname = None	# must exist in xml_pickle namespace, or thin-air
    return obj_from_name(classname, modname, paranoia)

def get_node_valuetext(node):
    "Get text from node, whether in value=, or in element body."

    # we know where the text is, based on whether there is
    # a value= attribute. ie. pickler can place it in either
    # place (based on user preference) and unpickler doesn't care

    if node._attrs.has_key('value'):
        # text in tag
        ttext = node.getAttribute('value')
        return unsafe_string(ttext)
    else:
        # text in body
        node.normalize()
        try:
            btext = node.childNodes[0].nodeValue
        except:
            btext = ''
        return unsafe_content(btext)

def _fix_family(family,typename):
    """
    If family is None or empty, guess family based on typename.
    (We can only guess for builtins, of course.)
    """
    
    if family and len(family):
        return family  # sometimes it's None, sometimes it's empty ...

    if typename == 'None':
        return 'none'
    if typename == 'dict':
        return 'map'
    elif typename == 'list':
        return 'seq'
    elif typename == 'tuple':
        return 'seq'
    elif typename == 'numeric':
        return 'atom'
    elif typename == 'string':
        return 'atom'
    elif typename == 'PyObject':
        return 'obj'
    elif typename == 'function':
        return 'lang'
    elif typename == 'class':
        return 'lang'
    elif typename == 'True':
        return 'uniq'
    elif typename == 'False':
        return 'uniq'
    else:
        raise XMLUnpicklingError, \
              "family= must be given for unknown type %s" % typename

def _thing_from_dom(dom_node, container=None, paranoia=1):
    "Converts an [xml_pickle] DOM tree to a 'native' Python object"
    for node in subnodes(dom_node):
        if node.nodeName == "PyObject":
            container = unpickle_instance(node, paranoia)
            # do we need to unmutate it? (check for type= being set --
            # will only be set for mutated objects)
            if node.getAttribute('type'):
                # get unmutator by type=
                klass = node.getAttribute('type')
                if mutate.can_unmutate(klass,container):
                    # note -- 'extra' isn't handled (yet) at the toplevel
                    container = mutate.unmutate(klass,container,paranoia,None)

            try:
                id = node.getAttribute('id')
                visited[id] = container
            except KeyError:
                pass

        elif node.nodeName in ['attr','item','key','val']:
            node_family = node.getAttribute('family')
            node_type = node.getAttribute('type')
            node_name = node.getAttribute('name')

            # check refid first (if present, type is type of referenced object)
            ref_id = node.getAttribute('refid')
                
            if len(ref_id):	 # might be empty or None
                if node.nodeName == 'attr':
                    setattr(container, node_name, visited[ref_id])
                else:
                    container.append(visited[ref_id])

                # done, skip rest of block
                continue

            # if we didn't find a family tag, guess (do after refid check --
            # old pickles will set type="ref" which _fix_family can't handle)
            node_family = _fix_family(node_family,node_type)

            node_valuetext = get_node_valuetext(node)

            # step 1 - set node_val to basic thing
            #if node_name == '__parent__' and getExcludeParentAttr():
            #	continue	# Do not pickle xml_objectify bookkeeping attribute
            if node_family == 'none':
                node_val = None
            elif node_family == 'atom':
                node_val = node_valuetext
            elif node_family == 'seq':
                # seq must exist in visited{} before we unpickle subitems,
                # in order to handle self-references
                seq = []
                _save_obj_with_id(node,seq)
                node_val = _thing_from_dom(node,seq,paranoia)
            elif node_family == 'map':
                # map must exist in visited{} before we unpickle subitems,
                # in order to handle self-references
                map = {}
                _save_obj_with_id(node,map)
                node_val = _thing_from_dom(node,map,paranoia)
            elif node_family == 'obj':
                node_val = unpickle_instance(node, paranoia)
            elif node_family == 'lang':
                # lang is a special type - we don't know how to unpickle
                # a generic 'lang' object, so we have to handle the specific
                # types here instead of in the block below
                #
                # In the future, this might be expanded to include
                # other languages, but for now, only Python is supported.
                if node_type == 'function':
                    node_val = unpickle_function(node.getAttribute('module'),
                                                 node.getAttribute('class'),
                                                 paranoia)
                elif node_type == 'class':
                    node_val = get_class_from_name(node.getAttribute('class'),
                                                   node.getAttribute('module'),
                                                   paranoia)
                else:
                    raise XMLUnpicklingError, "Unknown lang type %s" % node_type					
            elif node_family == 'uniq':
                # uniq is another special type that is handled here instead
                # of below.

                # Gnosis-1.0.6 encoded functions/classes as 'uniq' instead
                # of 'lang' -- accept these for backward compatibility.
                if node_type == 'function':
                    node_val = unpickle_function(node.getAttribute('module'),
                                                 node.getAttribute('class'),
                                                 paranoia)
                elif node_type == 'class':
                    node_val = get_class_from_name(node.getAttribute('class'),
                                                   node.getAttribute('module'),
                                                   paranoia)
                elif node_type == 'True':
                    node_val = TRUE_VALUE
                elif node_type == 'False':
                    node_val = FALSE_VALUE
                else:
                    raise XMLUnpicklingError, "Unknown uniq type %s" % node_type
            else:
                raise XMLUnpicklingError, "UNKNOWN family %s,%s,%s" % (node_family,node_type,node_name)

            # step 2 - take basic thing and make exact thing
            # Note there are several NOPs here since node_val has been decided
            # above for certain types. However, I left them in since I think it's
            # clearer to show all cases being handled (easier to see the pattern
            # when doing later maintenance).
            
            if node_type == 'None':
                node_val = None
            elif node_type == 'numeric':
                #node_val = safe_eval(node_val)
                node_val = to_number(node_val)
            elif node_type == 'string':
                node_val = node_val
            elif node_type == 'list':
                node_val = node_val
            elif node_type == 'tuple':
                # subtlety - if tuples could self-reference, this would be wrong
                # since the self ref points to a list, yet we're making it into
                # a tuple. it appears however that self-referencing tuples aren't
                # really all that legal (regular pickle can't handle them), so
                # this shouldn't be a practical problem.
                node_val = tuple(node_val)
            elif node_type == 'dict':
                node_val = node_val
            elif node_type == 'function':
                node_val = node_val
            elif node_type == 'class':
                node_val = node_val
            elif node_type == 'True':
                node_val = node_val
            elif node_type == 'False':
                node_val = node_val
            elif mutate.can_unmutate(node_type,node_val):
                mextra = node.getAttribute('extra')
                node_val = mutate.unmutate(node_type,node_val,paranoia,
                                           mextra)
            elif node_type == 'PyObject':
                node_val = node_val
            #elif ext.can_handle_xml(node_type,node_valuetext):
            #	node_val = ext.xml_to_obj(node_type, node_valuetext, paranoia)
            else:
                raise XMLUnpicklingError, "Unknown type %s,%s" % (node,node_type)

            if node.nodeName == 'attr':
                setattr(container,node_name,node_val)
            else:
                container.append(node_val)

            _save_obj_with_id(node,node_val)

        elif node.nodeName == 'entry':
            keyval = _thing_from_dom(node, [], paranoia)
            key, val = keyval[0], keyval[1]
            container[key] = val
            # <entry> has no id for refchecking

        else:
            raise XMLUnpicklingError, \
                  "element %s is not in PyObjects.dtd" % node.nodeName

    return container

