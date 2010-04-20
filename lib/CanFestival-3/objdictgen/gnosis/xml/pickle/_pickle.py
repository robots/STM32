"""Store Python objects to (pickle-like) XML Documents

Please see the information at gnosis.xml.pickle.doc for
explanation of usage, design, license, and other details
"""
from gnosis.xml.pickle.util import \
     _klass, _module, _EmptyClass, \
     safe_string, safe_content,\
     get_class_from_stack, get_class_full_search, \
     get_class_from_store, get_class_from_vapor, \
     getParanoia, getDeepCopy,	get_function_info, \
     getParser, getInBody, setInBody, getVerbose, enumParsers

from gnosis.xml.pickle.ext import can_mutate, mutate, \
     can_unmutate, unmutate, get_unmutator, try_mutate

from gnosis.util.introspect import isinstance_any, attr_dict, isInstanceLike, \
     hasCoreData, isNewStyleClass

from gnosis.util.XtoY import ntoa

# add "standard" extension types
import gnosis.xml.pickle.ext._mutators

# XML legality checking
from gnosis.xml.xmlmap import is_legal_xml

import gnosis.pyconfig

from types import *

try:		# Get a usable StringIO
    from cStringIO import StringIO
except:
    from StringIO import StringIO

# default settings
setInBody(IntType,0)
setInBody(FloatType,0)
setInBody(LongType,0)
setInBody(ComplexType,0)
setInBody(StringType,0)
# our unicode vs. "regular string" scheme relies on unicode
# strings only being in the body, so this is hardcoded.
setInBody(UnicodeType,1)

# Define exceptions and flags
XMLPicklingError = "gnosis.xml.pickle.XMLPicklingError"
XMLUnpicklingError = "gnosis.xml.pickle.XMLUnpicklingError"

# Maintain list of object identities for multiple and cyclical references
# (also to keep temporary objects alive)
visited = {}

# a multipurpose list-like object. it is nicer conceptually for us
# to pass lists around at the lower levels, yet we'd also like to be
# able to do things like write to a file without the overhead of building
# a huge list in memory first. this class handles that, yet drops in (for
# our purposes) in place of a list.
#
# (it's not based on UserList so that (a) we don't have to pull in UserList,
# and (b) it will break if someone accesses StreamWriter in an unexpected way
# rather than failing silently for some cases)
class StreamWriter:
    """A multipurpose stream object. Four styles:

    - write an uncompressed file
    - write a compressed file
    - create an uncompressed memory stream
    - create a compressed memory stream
        """
    def __init__(self, iohandle=None, compress=None):

        if iohandle: self.iohandle = iohandle
        else: self.iohandle = self.sio = StringIO()

        if compress == 1: # maybe we'll add more types someday
            import gzip
            self.iohandle = gzip.GzipFile(None,'wb',9,self.iohandle)

    def append(self,item):
        if type(item) in (ListType, TupleType): item = ''.join(item)
        self.iohandle.write(item)

    def getvalue(self):
        "Returns memory stream as a single string, or None for file objs"
        if hasattr(self,'sio'):
            if self.iohandle != self.sio:
                # if iohandle is a GzipFile, we need to close it to flush
                # remaining bits, write the CRC, etc. However, if iohandle is
                # the sio, we CAN'T close it (getvalue() wouldn't work)
                self.iohandle.close()
            return self.sio.getvalue()
        else:
            # don't raise an exception - want getvalue() unconditionally
            return None

# split off for future expansion of compression types, etc.
def StreamReader( stream ):
    """stream can be either a filehandle or string, and can
    be compressed/uncompressed. Will return either a fileobj
    appropriate for reading the stream."""

    # turn strings into stream
    if type(stream) in [StringType,UnicodeType]:
        stream = StringIO(stream)

    # determine if we have a gzipped stream by checking magic
    # number in stream header
    pos = stream.tell()
    magic = stream.read(2)
    stream.seek(pos)
    if magic == '\037\213':
        import gzip
        stream = gzip.GzipFile(None,'rb',None,stream)

    return stream

class XML_Pickler:
    """Framework for 'pickle to XML'.

    XML_Pickler offers a lot of flexibility in how you do your pickling.
    See the docs for examples.
    """
    def __init__(self, py_obj=None):
        if py_obj is not None:
            #if type(py_obj) is InstanceType:
            if isInstanceLike(py_obj):
                self.to_pickle = py_obj
            else:
                raise XMLPicklingError, \
                      "XML_Pickler must be initialized with Instance (or None)"

    def dump(self, iohandle, obj=None, binary=0, deepcopy=None):
        "Write the XML representation of obj to iohandle."
        # dumps() does all the work, but uses our filehandle
        self.dumps(obj,binary,deepcopy,iohandle)

    def load(self, fh, paranoia=None):
        "Load pickled object from file fh."
        # If paranoia is None, getParanoia() is used.
        global visited
        if paranoia is None: paranoia=getParanoia()

        # read from a file,compressed file,string,or compressed string
        fh = StreamReader(fh)

        visited = {}

        parser = enumParsers().get(getParser())
        if parser:
            return parser(fh, paranoia=paranoia)
        else:
            raise XMLUnpicklingError, "Unknown parser %s" % getParser()

    def dumps(self, obj=None, binary=0, deepcopy=None, iohandle=None):
        "Create the XML representation as a string."
        # If obj==None, pickle self. If deepcopy==None, getDeepCopy().
        if deepcopy is None: deepcopy = getDeepCopy()

        # write to a file or string, either compressed or not
        list = StreamWriter(iohandle,binary)

        # here are our three forms:
        if obj is not None:				# XML_Pickler().dumps(obj)
            return _pickle_toplevel_obj(list,obj, deepcopy)
        elif hasattr(self,'to_pickle'): # XML_Pickler(obj).dumps()
            return _pickle_toplevel_obj(list,self.to_pickle, deepcopy)
        else:							# myXML_Pickler().dumps()
            return _pickle_toplevel_obj(list,self, deepcopy)

    def loads(self, xml_str, paranoia=None):
        "Load a pickled object from the given XML string."
        # really just syntactic sugar for load()
        return self.load(xml_str,paranoia)

#-- support functions

def _pickle_toplevel_obj(xml_list, py_obj, deepcopy):
    "handle the top object -- add XML header, etc."

    # Store the ref id to the pickling object (if not deepcopying)
    global visited
    visited = {}
    if not deepcopy:
        id_ = id(py_obj)
        visited[id_] = py_obj

    # note -- setting family="obj" lets us know that a mutator was used on
    # the object. Otherwise, it's tricky to unpickle both <PyObject ...>
    # and <.. type="PyObject" ..> with the same code. Having family="obj" makes
    # it clear that we should slurp in a 'typeless' object and unmutate it.

    # note 2 -- need to add type= to <PyObject> when using mutators.
    # this is b/c a mutated object can still have a class= and
    # module= that we need to read before unmutating (i.e. the mutator
    # mutated into a PyObject)

    famtype = '' # unless we have to, don't add family= and type=

    #if type(py_obj) is not InstanceType:
    if not isInstanceLike(py_obj):
        # use our wrapper-mutator to pickle builtin types.
        # get by name since mutator classtype is None
        mutator = get_unmutator('builtin_wrapper',None)
        # wrapper must not have an id (otherwise the wrapper AND the
        # wrapper-obj would appear to have the same id)
        if not deepcopy:
            del visited[id_]
        id_ = None
        py_obj = mutator.mutate(py_obj).obj
        famtype = famtype + 'family="obj" type="%s" ' % mutator.tag
        # don't show module for wrapped types
        module = None
        # refs get funny here, but since this is a special case, and we're
        # only pickling a single object, turning deepcopy off is easiest
        #deepcopy = 1
    else:
        if can_mutate(py_obj):
            (mtype,py_obj,in_body,extra) = mutate(py_obj)
            # sanity check until/if we eventually support these
            # at the toplevel
            if in_body or extra:
                raise XMLPicklingError, \
                      "Sorry, mutators can't set in_body and/or extra at the toplevel."
            famtype = famtype + 'family="obj" type="%s" ' % mtype

        module = _module(py_obj)

    klass_tag = _klass(py_obj)

    # Generate the XML string
    if module: extra = '%smodule="%s" class="%s"' % (famtype,module,klass_tag)
    else:	   extra = '%s class="%s"' % (famtype,klass_tag)

    xml_list.append('<?xml version="1.0"?>\n'+
                    '<!DOCTYPE PyObject SYSTEM "PyObjects.dtd">\n')

    if deepcopy:
        xml_list.append('<PyObject %s>\n' % (extra))
    elif id_ is not None:
        xml_list.append('<PyObject %s id="%s">\n' % (extra, id_))
    else:
        xml_list.append('<PyObject %s>\n' % (extra))

    pickle_instance(py_obj, xml_list, level=0, deepcopy=deepcopy)
    xml_list.append('</PyObject>\n')

    # returns None if xml_list is a fileobj, but caller should
    # know that (or not care)
    return xml_list.getvalue()

def pickle_instance(obj, list, level=0, deepcopy=0):
    """Pickle the given object into a <PyObject>

    Add XML tags to list. Level is indentation (for aesthetic reasons)
    """
    # concept: to pickle an object, we pickle two things:
    #
    #	1. the object attributes (the "stuff")
    #	2. initargs, if defined
    #
    # There is a twist to this -- instead of always putting the "stuff"
    # into a container, we can make the elements of "stuff" first-level attributes,
    # which gives a more natural-looking XML representation of the object.
    #
    # We only put the "stuff" into a container if we'll need to pass it
    # later as an object to __setstate__.

    # tests below are in same order as pickle.py, in case someone depends on that.
    # note we save the special __getstate__ and __getinitargs__ objects in containers
    # of the same name -- we know for sure that the object can't also have
    # data attributes of that same name

    # first, get the initargs, if present
    try:
        args = obj.__getinitargs__()
        try:
            len(args)  # must be a sequence, from pickle.py
        except:
            raise XMLPicklingError, \
                  "__getinitargs__() must return a sequence"
    except:
        args = None

    # next, decide what the "stuff" is
    try:
        stuff = obj.__getstate__()
    except:
        #stuff = obj.__dict__
        stuff = attr_dict(obj)

    # save initargs, if we have them
    if args is not None:
        # put them in an <attr name="__getinitargs__" ...> container
        list.append(_attr_tag('__getinitargs__', args, level, deepcopy))

    # decide how to save the "stuff", depending on whether we need
    # to later grab it back as a single object
    if not hasattr(obj,'__setstate__'):
        if type(stuff) is DictType:
            # don't need it as a single object - save keys/vals as
            # first-level attributes
            for key,val in stuff.items():
                list.append(_attr_tag(key, val, level, deepcopy))
        else:
            raise XMLPicklingError, \
                  "__getstate__ must return a DictType here"
    else:
        # else, encapsulate the "stuff" in an <attr name="__getstate__" ...>
        list.append(_attr_tag('__getstate__', stuff, level, deepcopy))

#--- Functions to create XML output tags ---
def _attr_tag(name, thing, level=0, deepcopy=0):
    start_tag = '  '*level+('<attr name="%s" ' % name)
    close_tag ='  '*level+'</attr>\n'
    return _tag_completer(start_tag, thing, close_tag, level, deepcopy)

def _item_tag(thing, level=0, deepcopy=0):
    start_tag = '  '*level+'<item '
    close_tag ='  '*level+'</item>\n'
    return _tag_completer(start_tag, thing, close_tag, level, deepcopy)

def _entry_tag(key, val, level=0, deepcopy=0):
    start_tag = '  '*level+'<entry>\n'
    close_tag = '  '*level+'</entry>\n'
    start_key = '  '*level+'  <key '
    close_key = '  '*level+'  </key>\n'
    key_block = _tag_completer(start_key, key, close_key, level+1, deepcopy)
    start_val = '  '*level+'  <val '
    close_val = '  '*level+'  </val>\n'
    val_block = _tag_completer(start_val, val, close_val, level+1, deepcopy)
    return (start_tag + key_block + val_block + close_tag)

def _tag_compound(start_tag, family_type, thing, deepcopy, extra=''):
    """Make a start tag for a compound object, handling deepcopy & refs.
    Returns (start_tag,do_copy), with do_copy indicating whether a
    copy of the data is needed.
    """
    if deepcopy:
        # don't need ids in a deepcopied file (looks neater)
        start_tag = start_tag + '%s %s>\n' % (family_type,extra)
        return (start_tag, 1)
    else:
        if visited.get(id(thing)):
            start_tag = start_tag + '%s refid="%s" />\n' % (family_type,id(thing))
            return (start_tag, 0)
        else:
            start_tag = start_tag + '%s id="%s" %s>\n' % (family_type,id(thing),extra)
            return (start_tag, 1)

#
# This doesn't fit in any one place particularly well, but
# it needs to be documented somewhere. The following are the family
# types currently defined:
#
#	obj - thing with attributes and possibly coredata
#
#	uniq - unique thing, its type gives its value, and vice versa
#
#	map - thing that maps objects to other objects
#
#	seq - thing that holds a series of objects
#
#		   XXX - Py2.3 maybe the new 'Set' type should go here?
#
#	atom - non-unique thing without attributes (e.g. only coredata)
#
#	lang - thing that likely has meaning only in the
#		   host language (functions, classes).
#
#		   [Note that in Gnosis-1.0.6 and earlier, these were
#			mistakenly placed under 'uniq'. Those encodings are
#			still accepted by the parsers for compatibility.]
#

def _family_type(family,typename,mtype,mextra):
    """Create a type= string for an object, including family= if necessary.
    typename is the builtin type, mtype is the mutated type (or None for
    non-mutants). mextra is mutant-specific data, or None."""

    if getVerbose() == 0 and mtype is None:
        # family tags are technically only necessary for mutated types.
        # we can intuit family for builtin types.
        return 'type="%s"' % typename

    if mtype and len(mtype):
        if mextra:
            mextra = 'extra="%s"' % mextra
        else:
            mextra = ''
        return 'family="%s" type="%s" %s' % (family,mtype,mextra)
    else:
        return 'family="%s" type="%s"' % (family,typename)

# sanity in case Python changes ...
if gnosis.pyconfig.Have_BoolClass() and gnosis.pyconfig.IsLegal_BaseClass('bool'):
    raise XMLPicklingError, \
          "Assumption broken - can now use bool as baseclass!"

Have_BoolClass = gnosis.pyconfig.Have_BoolClass()

def _tag_completer(start_tag, orig_thing, close_tag, level, deepcopy):
    tag_body = []

    (mtag,thing,in_body,mextra) = try_mutate(orig_thing,None,
                                             getInBody(type(orig_thing)), None)

    if type(thing) is NoneType:
        start_tag = start_tag + "%s />\n" % (_family_type('none','None',None,None))
        close_tag = ''
    # bool cannot be used as a base class (see sanity check above) so if thing
    # is a bool it will always be BooleanType, and either True or False
    elif Have_BoolClass and type(thing) is BooleanType:
        if thing is True:
            typestr = 'True'
        else: # must be False
            typestr = 'False'

        if in_body:
            start_tag = start_tag + '%s>%s' % \
                      (_family_type('uniq',typestr,mtag,mextra),
                       '')
            close_tag = close_tag.lstrip()
        else:
            start_tag = start_tag + '%s value="%s" />\n' % \
                      (_family_type('uniq',typestr,mtag,mextra),
                       '')
            close_tag = ''
    # ClassType will get caught by isInstanceLike(), which is not
    # what we want. There are two cases here - the first catches
    # old-style classes, the second catches new-style classes.
    elif isinstance(thing,ClassType) or isNewStyleClass(thing):
        module = thing.__module__
        if module:
            extra = 'module="%s" class="%s"' % (module, thing.__name__)
        else:
            extra = 'class="%s"' % _klass(thing.__name__)
        start_tag = start_tag + '%s %s/>\n' % \
                     (_family_type('lang','class',mtag,mextra),extra)

        close_tag = ''
    # have to check for instance-like next since ints, etc., can be
    # instance-like in Python 2.2+. if it's really an object, we don't
    # want to fall through to the regular int,float,etc. code, since
    # that would skip the special handling in pickle_instance().
    elif isInstanceLike(thing):
        module = _module(thing)
        if module:
            extra = 'module="%s" class="%s"' % (module, _klass(thing))
        else:
            extra = 'class="%s"' % _klass(thing)
        start_tag, do_copy = \
                   _tag_compound(start_tag,_family_type('obj','PyObject',mtag,mextra),
                                 orig_thing, deepcopy, extra)
        # need to remember we've seen container before pickling subitems
        visited[id(orig_thing)] = orig_thing
        if do_copy:
            pickle_instance(thing, tag_body, level+1, deepcopy)
        else:
            close_tag = ''
    elif isinstance_any(thing, (IntType, LongType, FloatType, ComplexType)):
        #thing_str = repr(thing)
        thing_str = ntoa(thing)

        if in_body:
            # we don't call safe_content() here since numerics won't
            # contain special XML chars.
            # the unpickler can either call unsafe_content() or not,
            # it won't matter
            start_tag = start_tag + '%s>%s' % \
                      (_family_type('atom','numeric',mtag,mextra),
                       thing_str)
            close_tag = close_tag.lstrip()
        else:
            start_tag = start_tag + '%s value="%s" />\n' % \
                      (_family_type('atom','numeric',mtag,mextra),thing_str)
            close_tag = ''
    elif isinstance_any(thing, (StringType,UnicodeType)):
        #XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
        # special check for now - this will be fixed in the next major
        # gnosis release, so I don't care that the code is inline & gross
        # for now
        #XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX       
        if isinstance(thing,UnicodeType):
            # can't pickle unicode containing the special "escape" sequence
            # we use for putting strings in the XML body (they'll be unpickled
            # as strings, not unicode, if we do!)
            if thing[0:2] == u'\xbb\xbb' and thing[-2:] == u'\xab\xab':
                raise Exception("Unpickleable Unicode value. To be fixed in next major Gnosis release.")
        
            # see if it contains any XML-illegal values
            if not is_legal_xml(thing):
                raise Exception("Unpickleable Unicode value. To be fixed in next major Gnosis release.")

        if isinstance(thing,StringType) and getInBody(StringType):
            # technically, this will crash safe_content(), but I prefer to
            # have the test here for clarity
            try:
                # safe_content assumes it can always convert the string
                # to unicode, which isn't true (eg. pickle a UTF-8 value)
                u = unicode(thing)
            except:
                raise Exception("Unpickleable string value (%s). To be fixed in next major Gnosis release." % repr(thing))

        #XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
        # End of temporary hack code
        #XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
            
        if in_body:
            start_tag = start_tag + '%s>%s' % \
                      (_family_type('atom','string',mtag,mextra),
                       safe_content(thing))
            close_tag = close_tag.lstrip()
        else:
            start_tag = start_tag + '%s value="%s" />\n' % \
                      (_family_type('atom','string',mtag,mextra),
                       safe_string(thing))
            close_tag = ''
    # General notes:
    #	1. When we make references, set type to referenced object
    #	   type -- we don't need type when unpickling, but it may be useful
    #	   to someone reading the XML file
    #	2. For containers, we have to stick the container into visited{}
    #	   before pickling subitems, in case it contains self-references
    #	   (we CANNOT just move the visited{} update to the top of this
    #	   function, since that would screw up every _family_type() call)
    elif type(thing) is TupleType:
        start_tag, do_copy = \
                   _tag_compound(start_tag,_family_type('seq','tuple',mtag,mextra),
                                 orig_thing,deepcopy)
        if do_copy:
            for item in thing:
                tag_body.append(_item_tag(item, level+1, deepcopy))
        else:
            close_tag = ''
    elif type(thing) is ListType:
        start_tag, do_copy = \
                   _tag_compound(start_tag,_family_type('seq','list',mtag,mextra),
                                 orig_thing,deepcopy)
        # need to remember we've seen container before pickling subitems
        visited[id(orig_thing)] = orig_thing
        if do_copy:
            for item in thing:
                tag_body.append(_item_tag(item, level+1, deepcopy))
        else:
            close_tag = ''
    elif type(thing) in [DictType]:
        start_tag, do_copy = \
                   _tag_compound(start_tag,_family_type('map','dict',mtag,mextra),
                                 orig_thing,deepcopy)
        # need to remember we've seen container before pickling subitems
        visited[id(orig_thing)] = orig_thing
        if do_copy:
            for key, val in thing.items():
                tag_body.append(_entry_tag(key, val, level+1, deepcopy))
        else:
            close_tag = ''
    elif type(thing) in [FunctionType,BuiltinFunctionType]:
        info = get_function_info(thing)
        # use module/class tags -- not perfect semantically, but better
        # that creating new attr names
        start_tag = start_tag + '%s module="%s" class="%s"/>\n' % \
                     (_family_type('lang','function',mtag,mextra),
                      info[0],info[1])
        close_tag = ''
    else:
        # try using pickled value as the XML value tag.
        # rationale:  it won't be (easily) editable, but at least
        # you'll get valid XML even if your classes happen to
        # contain a few "foreign" types, and you don't feel like
        # writing a helper object (see gnosis.xml.pickle.ext for
        # how to do that)
        try:
            # we can't lookup the helper by type, since rawpickle can pickle
            # any pickleable class, so lookup by tag (unmutator) instead
            # (mutator & unmutator are always the same object)

            # always put rawpickles in the element body
            mutator = get_unmutator('rawpickle',None)
            thing = safe_content(mutator.mutate(thing).obj)
            start_tag = start_tag + '%s>%s' % (_family_type('atom',None,'rawpickle',None),
                                   thing)
            close_tag = close_tag.lstrip()
        except:
            raise XMLPicklingError, "non-handled type %s" % type(thing)

    # need to keep a ref to the object for two reasons -
    #  1. we can ref it later instead of copying it into the XML stream
    #  2. need to keep temporary objects around so their ids don't get reused

    # if DEEPCOPY, we can skip this -- reusing ids is not an issue if we
    # never look at them
    if not deepcopy:
        visited[id(orig_thing)] = orig_thing

    return start_tag + ''.join(tag_body) + close_tag

# pickle-compatible API, not exported by default (note we have to
# reverse the args in dump() -- we can't reverse the XML_Pickler.dump()
# args without losing functionality)

dump = lambda o,f,b=0: XML_Pickler().dump(f,o,b)
dumps = lambda o,b=0: XML_Pickler().dumps(o,b)
loads = XML_Pickler().loads
load = XML_Pickler().load

