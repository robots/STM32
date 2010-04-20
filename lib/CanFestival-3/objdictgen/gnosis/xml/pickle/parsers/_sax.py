"""Pure-Python SAX parser for xml_pickle"""

__author__ = "Frank McIngvale (frankm@hiwaay.net)"

# _sax.py has higher requirements for installed packages than
# _dom.py, so _pickle checks for errors during importing
# and turns off SAX support if errors occur

from xml.sax.expatreader import ExpatParser
from xml.sax.handler import ContentHandler, ErrorHandler
from xml.sax.xmlreader import InputSource
from gnosis.xml.pickle.util import obj_from_name, unpickle_function, \
     get_class_from_name, unsafe_string, unsafe_content
from gnosis.util.introspect import attr_update
import gnosis.xml.pickle.ext as mutate
from gnosis.xml.pickle.parsers import _dom
from gnosis.xml.pickle.parsers._dom import TRUE_VALUE,FALSE_VALUE
from gnosis.util.XtoY import to_number

import sys, os, string
from types import *
from StringIO import StringIO

# Define exceptions and flags
XMLPicklingError = "gnosis.xml.pickle.XMLPicklingError"
XMLUnpicklingError = "gnosis.xml.pickle.XMLUnpicklingError"

DEBUG = 0

def dbg(msg,force=0):
    if DEBUG or force:
        print msg

class _EmptyClass: pass

class xmlpickle_handler(ContentHandler):

    def __init__(self,paranoia=1):
        self.paranoia = paranoia

    # do the __init__ stuff here
    def startDocument(self):
        # raw elements are placed here when we get a startElement event.
        self.elem_stk = []

        # on endElement events, we pop a raw item from elem_stk,
        # reduce it, and place the object on val_stk
        # (container objects then grab their subitems from val_stk)
        self.val_stk = []

        self.visited = {}
        self.nr_objs = 0

        # optimization -- technically we need to put text as nodes on
        # the stack along with elements, however, we know that we'll
        # only get text between non-nested tags, so we can keep a
        # single string
        self.content = ""

    def getobj(self):
        "return final object"
        return self.val_stk[-1][2]

    def prstk(self,force=0):
        if DEBUG == 0 and not force:
            return
        print "**ELEM STACK**"
        for i in self.elem_stk:
            print str(i)
        print "**VALUE STACK**"
        for i in self.val_stk:
            print str(i)

    def save_obj_id(self,obj,elem):

        id = elem[4].get('id')
        if id: # not all tags have an id=
            #dbg("SAVING REF %s %s"%(str(id),obj))
            self.visited[id] = obj

    def pop_stateinfo(self):
        # look back in stack till we either hit STOP or
        # find an attr __getstate__
        i = -1
        while self.val_stk[i][0] != 'STOP':
            if self.val_stk[i][0] == 'attr' and \
               self.val_stk[i][1] == '__getstate__':
                info = self.val_stk.pop(i)[2]
                return info
            i -= 1

        return None

    def pop_initargs(self):
        # look back in stack till we either hit STOP or
        # find an attr __getinitargs__
        i = -1
        while self.val_stk[i][0] != 'STOP':
            if self.val_stk[i][0] == 'attr' and \
               self.val_stk[i][1] == '__getinitargs__':
                info = self.val_stk.pop(i)[2]
                return info
            i -= 1

        return None

    def unpickle_instance(self,elem):

        obj = elem[5]

        # call __init__ if initargs exist
        args = self.pop_initargs()
        if hasattr(obj,'__init__') and args is not None:
            apply(obj.__init__,args)

        # get stateinfo if exists
        state = self.pop_stateinfo()

        # set obj attributes
        if state:
            if hasattr(obj,'__setstate__'):
                obj.__setstate__(state)
            else:
                #obj.__dict__.update(state)
                attr_update(obj, state)
        else:
            while self.val_stk[-1][0] != 'STOP':
                e = self.val_stk.pop()
                setattr(obj,e[1],e[2])

        self.val_stk.pop() # pop STOP

        return obj

    def reduce(self,name,bodytext):

        #dbg("ABOUT TO REDUCE %s %s"%(name,str(self.elem_stk[-1])))
        dbg("ABOUT TO REDUCE %s"%(name))
        self.prstk()

        elem = self.elem_stk.pop()

        refid = elem[4].get('refid')
        if refid:
            obj = self.visited[refid]
            #dbg("GOT REF %s,%s"%(str(refid),obj))
            self.val_stk.append((elem[0],elem[3],obj))
            dbg("<<<<<<<<<<<< REDUCED")
            self.prstk()
            self.nr_objs += 1
            return

        if elem[4].get('value'):
            valuetext = unsafe_string(elem[4].get('value'))
        else:
            valuetext = unsafe_content(bodytext)

        #valuetext = elem[4].get('value') or bodytext

        if name in ['attr','key','val','item']:

            # step 1 -- convert family -> basic_type
            family = elem[1]

            if family == 'atom':
                dbg("** ATOM")
                obj = valuetext

            elif family == 'none':
                dbg("** NONE")
                obj = None

            elif family == 'seq':
                dbg("** SEQ")
                obj = elem[5]
                while self.val_stk[-1][0] != 'STOP':
                    obj.insert(0,self.val_stk.pop()[2])

                self.val_stk.pop() # pop STOP

            elif family == 'map':
                dbg("** MAP")
                obj = elem[5]
                while self.val_stk[-1][0] != 'STOP':
                    e = self.val_stk.pop()
                    obj[e[2][0]] = e[2][1]

                self.val_stk.pop()  # pop STOP

            elif family == 'obj':
                dbg("** OBJ")
                obj = self.unpickle_instance(elem)

            elif family == 'lang':
                if elem[2] == 'function':
                    obj = unpickle_function(elem[4].get('module'),
                                            elem[4].get('class'),
                                            self.paranoia)
                elif elem[2] == 'class':
                    obj = get_class_from_name(elem[4].get('class'),
                                              elem[4].get('module'),
                                              self.paranoia)
                else:
                    raise XMLUnpicklingError, \
                          "Unknown lang type %s" % elem[2]
                
            elif family == 'uniq':
                # uniq is a special type - we don't know how to unpickle
                # a generic 'uniq' object, so we have to handle the specific
                # types here instead of in the block below

                # Gnosis-1.0.6 and earlier encoded functions/classes
                # as 'uniq' instead of 'lang' -- accept those encodings
                # for backward compatibility
                if elem[2] == 'function':
                    obj = unpickle_function(elem[4].get('module'),
                                            elem[4].get('class'),
                                            self.paranoia)
                elif elem[2] == 'class':
                    obj = get_class_from_name(elem[4].get('class'),
                                              elem[4].get('module'),
                                              self.paranoia)
                elif elem[2] == 'True':
                    obj = TRUE_VALUE
                elif elem[2] == 'False':
                    obj = FALSE_VALUE
                else:
                    raise XMLUnpicklingError, \
                          "Unknown uniq type %s" % elem[2]
            else:
                raise XMLUnpicklingError, \
                      "UNKNOWN family %s,%s,%s" % \
                      (family,elem[2],elem[3])

            # step 2 -- convert basic -> specific type
            # (many of these are NOPs, but included for clarity)

            if elem[2] == 'numeric':
                dbg("** NUMERIC")

                #if obj.isdigit():
                #   obj = string.atoi(obj)
                #else:
                #   obj = eval(obj)
                obj = to_number(obj)

            elif elem[2] == 'string':
                dbg("** STRING")
                obj = obj

            elif elem[2] == 'None':
                obj = obj

            elif elem[2] in ['tuple','list']:
                dbg("** TUPLE/LIST")

                if elem[2] != 'list':
                    obj = tuple(obj)
                else:
                    obj = obj

            elif elem[2] == 'dict':
                obj = obj

            elif elem[2] == 'PyObject':
                obj = obj

            elif elem[2] == 'function':
                obj = obj

            elif elem[2] == 'class':
                obj = obj

            elif elem[2] == 'True':
                obj = obj

            elif elem[2] == 'False':
                obj = obj
                
            elif mutate.can_unmutate(elem[2],obj):
                mextra = elem[4].get('extra')
                obj = mutate.unmutate(elem[2],obj,self.paranoia,mextra)

            #elif xmlext.can_handle_xml(elem[1],valuetext):
            #   obj = xmlext.xml_to_obj(elem[1],valuetext,self.paranoia)

            else:
                self.prstk(1)
                raise XMLUnpicklingError, \
                      "UNHANDLED elem %s"%elem[2]

            # push on stack and save obj ref
            self.val_stk.append((elem[0],elem[3],obj))
            self.save_obj_id(obj,elem)
            self.nr_objs += 1

        elif name == 'entry':
            e1 = self.val_stk.pop()
            e2 = self.val_stk.pop()
            if e1[0] == 'val':
                ent = ((elem[0],elem[3],(e2[2],e1[2])))
            else:
                ent = ((elem[0],elem[3],(e1[2],e2[2])))

            # <entry> actually has no id
            self.save_obj_id(ent,elem)

            self.val_stk.append(ent)

        elif name == 'PyObject':
            obj = self.unpickle_instance(elem)

            # do we need to unmutate it? (check for type= being set -
            # will only be set for mutated objects)
            if elem[2] is not None and len(elem[2]):
                if mutate.can_unmutate(elem[2],obj):
                    # note -- 'extra' isn't handled (yet) at the toplevel
                    obj = mutate.unmutate(elem[2],obj,self.paranoia,None)

            self.val_stk.append((elem[0],elem[3],obj))
            self.save_obj_id(obj,elem)
            self.nr_objs += 1

        else: raise str("UNHANDLED name %s"%name)

        dbg("<<<<<<<<<<<< REDUCED")
        self.prstk()

    def endDocument(self):
        if DEBUG == 1:
            print "NROBJS "+str(self.nr_objs)

    def startElement(self,name,attrs):
        dbg("** START ELEM %s,%s"%(name,attrs._attrs))
        # push on elem_stk as tuple:
        #  (name,family,type,attr_name,attrs,[container]?)
        # name = attr/item/key/val/etc.
        # family = normalized family name
        # type = list/dict/string/etc.
        # attr_name = attribute name=, or none
        # attrs = dict of all tag attributes
        # container = for container types, the empty container to
        #             be filled
        node_type = attrs._attrs.get('type')

        # determine object family
        family = attrs._attrs.get('family')
        if node_type is None and name == 'PyObject':
            # <PyObject> doesn't have a "type=" if not mutated
            family = 'obj'
        elif name == 'entry':
            # entries don't need family=
            family = ''
        else:
            family = _dom._fix_family(family,node_type)

        if not attrs._attrs.get('refid'):
            # we don't set a container for <entry> -- also see below
            if family == 'seq':
                container = []
            elif family == 'map':
                container = {}
            elif family == 'obj' or name == 'PyObject':
                container = obj_from_name(attrs._attrs.get('class'),
                                          attrs._attrs.get('module',None),
                                          self.paranoia)
            else:
                container = None
        else: # don't want container for refs
            container = None

        elem = (name, family, node_type, attrs._attrs.get('name'),
                attrs._attrs, container)

        if container is not None:
            # have to save ref to container before we start grabbing subitems,
            # in case the subitems reference the container.
            self.save_obj_id(container,elem)

        self.elem_stk.append(elem)

        # for each "container" type, we place a STOP token
        # on the val_stk we'll know exactly where the elements
        # begin and end -- maybe there's a way to avoid this,
        # but this keeps things simple (otherwise, given for example
        #   (1,(2,3,4),(5,6)), you get a string of <items> and
        #   it's hard to tell which belongs where)
        #
        # (don't need a STOP token for <entry>, since it always
        # holds a single key/value pair)

        if container is not None:
            if not attrs._attrs.get('refid'):
                self.val_stk.append(("STOP",))

        # reset content
        self.content = ''

        self.prstk()

    def endElement(self,name):
        self.reduce(name,self.content)

    def characters(self,content):
        self.content += content

    # implement the ErrorHandler interface here as well
    def error(self,exception):
        print "** ERROR - dumping stacks"
        self.prstk(1)
        raise exception

    def fatalError(self,exception):
        print "** FATAL ERROR - dumping stacks"
        self.prstk(1)
        raise exception

    def warning(self,exception):
        print "WARNING"
        raise exception

    # Implement EntityResolver interface (called when the parser runs
    # across external entities - in our case, PyObjects.dtd).
    # For now we just ignore - we're using expat which is a non-validating
    # parser anyways (apparently xmlproc will validate, but expat is
    # chosen for speed). If we want to do validation in the future, we
    # can update this.
    def resolveEntity(self,publicId,systemId):
       inp = InputSource()
       inp.setByteStream(StringIO(""))
       return inp


# entry point expected by XML_Pickle
def thing_from_sax(filehandle=None,paranoia=1):

    if DEBUG == 1:
        print "**** SAX PARSER ****"

    e = ExpatParser()
    m = xmlpickle_handler(paranoia)
    e.setContentHandler(m)
    e.setErrorHandler(m)
    e.setEntityResolver(m)

    if filehandle:
        e.parse(filehandle)
    else:
        raise "Must pass a fileobj"

    return m.getobj()

