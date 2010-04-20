"""Utilities for working with gnosis.xml.objectify objects

Please see the information at gnosis.xml.objectify.doc for additional
explanation of usage, design, license, and other details
"""
from __future__ import generators
from gnosis.xml.objectify._objectify import _XO_
from gnosis.xml.objectify._objectify import *
from exceptions import TypeError
from types import *
from itertools import islice
from sys import maxint, stdout

def addChild(parent, child):
    "Add a child xmlObject to a parent xmlObject"
    name = tagname(child)
    if hasattr(parent, name):  # If exists add, make into list if needed
       tag = getattr(parent, name)
       if type(tag) is not list:
          setattr(parent, name, [tag])
          tag = [tag]
       tag.append(child)
    else:                      # Create a new one
       setattr(parent, name, child)
       if not parent._seq: parent._seq = []
    parent._seq.append(child)  # All tags end up in _seq

def walk_xo(o):
    "Recursively traverse the nodes of an _XO_ tree (depth first)"
    yield o
    for node in children(o):
        for child in walk_xo(node):
            yield child

def write_xml(o, out=stdout):
    "Serialize an _XO_ object back into XML"
    out.write("<%s" % tagname(o))
    for attr in attributes(o):
        out.write(' %s=%s' % attr)
    out.write('>')
    for node in content(o):
        if type(node) in StringTypes:
            out.write(node)
        else:
            write_xml(node, out=out)
    out.write("</%s>" % tagname(o))

def XPath(o, path):
    "Find node(s) within an _XO_ object"
    if not isinstance(o,_XO_):
        raise TypeError, \
              "XPath() only defined on gnosis.xml.objectify._XO_ object"
    path = path.replace('//','/!!') # Placeholder hack for easy splitting
    if path.startswith('/'):        # No need for init / since node==root
        path = path[1:]
    if path.startswith('!!'):       # Recursive path fragment
        path, start, stop = indices(path)
        i = 0
        for match in walk_xo(o):
            if i >= stop: return
            for node in XPath(match, path[2:]):
                if start <= i < stop:
                    yield node
                i += 1
    elif '/' in path[1:]:           # Compound, non-recursive
        head, tail = path.split('/', 1)
        for match in XPath(o, head):
            for node in XPath(match, tail):
                yield node
    else:                           # Atomic path fragment
        path, start, stop = indices(path)
        if path=="*":               # Node wildcard
            for node in islice(children(o), start, stop):
                yield node
        elif path=="text()":        # Node text(s)
            for s in islice(text(o), start, stop):
                yield s
        elif path.startswith('@*'): # All node attributes
            for attr in attributes(o):
                yield attr
        elif path.startswith('@'):  # Specific node attribute
            for attr in attributes(o):
                if attr[0]==path[1:]:
                    yield attr
        elif hasattr(o, path):      # Named node type
            for node in islice(getattr(o, path), start, stop):
                yield node

def indices(path):
    if '[' in path:                 # Check for indices
        path, param = path[:-1].split('[')
        slice_ = map(int, param.split('..'))
        start = slice_[0]-1
        if len(slice_) == 2:
            stop = slice_[1]
        else:
            stop = start+1
    else:
        start, stop = 0, maxint
    return path, start, stop

def _dir(o):
    try:    return o.__dict__.keys()
    except: return []

#-- Self-test utility functions
def pyobj_printer(py_obj, level=0):
    "Return a 'deep' string description of a Python object"
    if level==0: descript = '-----* '+py_obj.__class__.__name__+' *-----\n'
    else: descript = ''
    if hasattr(py_obj, '_XML'):     # present the literal XML of object
        prettified_XML = ' '.join(py_obj._XML.split())[:50]
        descript = (' '*level)+'CONTENT='+prettified_XML+'...\n'
    else:                           # present the object hierarchy view
        for membname in _dir(py_obj):
            if membname in ("__parent__", "_seq"):
               continue             # ExpatFactory uses bookeeping attribute
            member = getattr(py_obj,membname)
            if type(member) == InstanceType:
                descript += '\n'+(' '*level)+'{'+membname+'}\n'
                descript += pyobj_printer(member, level+3)
            elif type(member) == ListType:
                for i in range(len(member)):
                    descript += '\n'+(' '*level)+'['+membname+'] #'+str(i+1)
                    descript += (' '*level)+'\n'+pyobj_printer(member[i],level+3)
            else:
                descript += (' '*level)+membname+'='
                memval = ' '.join(unicode(member).split())
                if len(memval) > 50:
                    descript += memval[:50]+'...\n'
                else:
                    descript += memval + '\n'
    return descript
