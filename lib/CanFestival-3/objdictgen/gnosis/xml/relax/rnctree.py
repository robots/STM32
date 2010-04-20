#!/usr/bin/env python
# Convert an RELAX NG compact syntax schema to a Node tree
# This file released to the Public Domain by David Mertz
from __future__ import generators
import sys
from rnc_tokenize import token_list

class ParseError(SyntaxError): pass

for t in """
  ANY SOME MAYBE ONE BODY ANNOTATION ELEM ATTR GROUP LITERAL
  NAME COMMENT TEXT EMPTY INTERLEAVE CHOICE SEQ ROOT
  DEFAULT_NS NS DATATYPES DATATAG PATTERN START DEFINE
  """.split(): globals()[t] = t

PAIRS = {'BEG_BODY':('END_BODY',BODY),
         'BEG_PAREN':('END_PAREN',GROUP),
         'BEG_ANNO':('END_ANNO',ANNOTATION)}

TAGS = { ONE:   'group',
         SOME:  'oneOrMore',
         MAYBE: 'optional',
         ANY:   'zeroOrMore'}

DEFAULT_NAMESPACE = None
DATATYPE_LIB = [0, '"http://www.w3.org/2001/XMLSchema-datatypes"']
OTHER_NAMESPACE = {}
CONTEXT_FREE = 0

try: enumerate
except: enumerate = lambda seq: zip(range(len(seq)),seq)
nodetypes = lambda nl: tuple(map(lambda n: n.type, nl))
toNodes = lambda toks: map(lambda t: Node(t.type, t.value), toks)

class Node(object):
    __slots__ = ('type','value','name','quant')
    def __iter__(self): yield self
    __len__ = lambda self: 1

    def __init__(self, type='', value=[], name=None, quant=ONE):
        self.type  = type
        self.value = value
        self.name  = name
        self.quant = quant

    def format(self, indent=0):
        out = ['  '*indent+repr(self)]
        write = out.append
        if isinstance(self.value, str):
            if self.type==COMMENT:
                write('  '*(1+indent)+self.value)
        else:
            for node in self.value:
                write(node.format(indent+1))
        return '\n'.join(out)

    def prettyprint(self):
        print self.format()

    def toxml(self):
        if CONTEXT_FREE:
            out = []
            write = out.append
            write('<?xml version="1.0" encoding="UTF-8"?>')
            write('<grammar>')
            self.type = None
            write(self.xmlnode(1))
            write('</grammar>')
            return self.add_ns('\n'.join(out))
        else:
            return self.add_ns(self.xmlnode())


    def xmlnode(self, indent=0):
        out = []
        write = out.append
        if self.type == ROOT:
            write('<?xml version="1.0" encoding="UTF-8"?>')

        for x in self.value:
            if not isinstance(x, Node):
                raise TypeError, "Unhappy Node.value: "+repr(x)
            elif x.type == START:
                startelem = '<start><ref name="%s"/></start>' % x.value
                write('  '*indent+startelem)
            elif x.type == DEFINE:
                write('  '*indent+'<define name="%s">' % x.name)
                write(x.xmlnode(indent+1))
                write('  '*indent+'</define>')
            elif x.type == NAME:
                write('  '*indent+ '<ref name="%s"/>' % x.value)
            elif x.type == COMMENT:
                write('  '*indent+'<!-- %s -->' % x.value)
            elif x.type == LITERAL:
                write('  '*indent+'<value>%s</value>' % x.value)
            elif x.type == ANNOTATION:
                write('  '*indent+\
                      '<a:documentation>%s</a:documentation>' % x.value)
            elif x.type == INTERLEAVE:
                write('  '*indent+'<interleave>')
                write(x.xmlnode(indent+1))
                write('  '*indent+'</interleave>')
            elif x.type == CHOICE:
                write('  '*indent+'<choice>')
                write(x.xmlnode(indent+1))
                write('  '*indent+'</choice>')
            elif x.type == GROUP:
                write(x.xmlnode(indent))
            elif x.type == TEXT:
                write('  '*indent+'<text/>')
            elif x.type == EMPTY:
                write('  '*indent+'<empty/>')
            elif x.type == DATATAG:
                DATATYPE_LIB[0] = 1     # Use datatypes
                if x.name is None:      # no paramaters
                    write('  '*indent+'<data type="%s"/>' % x.value)
                else:
                    write('  '*indent+'<data type="%s">' % x.name)
                    p = '<param name="pattern">%s</param>' % x.value
                    write('  '*(indent+1)+p)
                    write('  '*indent+'</data>')
            elif x.type == ELEM:
                if x.quant == ONE:
                    write('  '*indent+'<element name="%s">' % x.name)
                    write(x.xmlnode(indent+1))
                    write('  '*indent+'</element>')
                else:
                    write('  '*indent+'<%s>' % TAGS[x.quant])
                    write('  '*(indent+1)+'<element name="%s">' % x.name)
                    write(x.xmlnode(indent+2))
                    write('  '*(indent+1)+'</element>')
                    write('  '*indent+'</%s>' % TAGS[x.quant])
            elif x.type == ATTR:
                if x.value[0].type == TEXT:
                    write('  '*indent+'<attribute name="%s"/>' % x.name)
                elif x.value[0].type == EMPTY:
                    write('  '*indent+'<attribute name="%s">' % x.name)
                    write('  '*(indent+1)+'<empty/>')
                    write('  '*indent+'</attribute>')

        return '\n'.join(out)

    def __repr__(self):
        return "Node(%s,%s,%s)[%d]" % (self.type, self.name,
                                       self.quant, len(self.value))

    def add_ns(self, xml):
        "Add namespace attributes to top level element"
        lines = xml.split('\n')
        self.nest_annotations(lines) # annots not allowed before root elem
        for i, line in enumerate(lines):
            ltpos = line.find('<')
            if ltpos >= 0 and line[ltpos+1] not in ('!','?'):
                # We've got an element tag, not PI or comment
                new = line[:line.find('>')]
                new += ' xmlns="http://relaxng.org/ns/structure/1.0"'
                if DEFAULT_NAMESPACE is not None:
                    new += '\n    ns=%s' % DEFAULT_NAMESPACE
                if DATATYPE_LIB[0]:
                    new += '\n    datatypeLibrary=%s' % DATATYPE_LIB[1]
                for ns, url in OTHER_NAMESPACE.items():
                    new += '\n    xmlns:%s=%s' % (ns, url)
                new += '>'
                lines[i] = new
                break
        return '\n'.join(lines)

    def nest_annotations(self, lines):
        "Nest any top annotation within first element"
        top_annotations = []
        for i, line in enumerate(lines[:]):
            if line.find('<a:') >= 0:
                top_annotations.append(line)
                del lines[i]
            else:
                ltpos = line.find('<')
                if ltpos >= 0 and line[ltpos+1] not in ('!','?'):
                    break
        for line in top_annotations:
            lines.insert(i, '  '+line)

def findmatch(beg, nodes, offset):
    level = 1
    end = PAIRS[beg][0]
    for i,t in enumerate(nodes[offset:]):
        if t.type == beg:   level += 1
        elif t.type == end: level -= 1
        if level == 0:
            return i+offset
    raise EOFError, ("No closing token encountered for %s @ %d"
                      % (beg,offset))

def match_pairs(nodes):
    newnodes = []
    i = 0
    while 1:
        if i >= len(nodes): break
        node = nodes[i]
        if node.type in PAIRS.keys():
            # Look for enclosing brackets
            match = findmatch(node.type, nodes, i+1)
            matchtype = PAIRS[node.type][1]
            node = Node(type=matchtype, value=nodes[i+1:match])
            node.value = match_pairs(node.value)
            newnodes.append(node)
            i = match+1
        else:
            newnodes.append(node)
            i += 1
        if i >= len(nodes): break
        if nodes[i].type in (ANY, SOME, MAYBE):
            newnodes[-1].quant = nodes[i].type
            i += 1
    nodes[:] = newnodes
    return nodes

def type_bodies(nodes):
    newnodes = []
    i = 0
    while 1:
        if i >= len(nodes): break
        if nodetypes(nodes[i:i+3]) == (ELEM, NAME, BODY) or \
           nodetypes(nodes[i:i+3]) == (ATTR, NAME, BODY):
            name, body = nodes[i+1].value, nodes[i+2]
            value, quant = type_bodies(body.value), body.quant
            node = Node(nodes[i].type, value, name, quant)
            newnodes.append(node)
            i += 3
        elif nodetypes(nodes[i:i+2]) == (DATATAG, PATTERN):
            node = Node(DATATAG, nodes[i+1].value, nodes[i].value)
            newnodes.append(node)
            i += 2
        elif nodes[i] == DEFINE:
            print nodes[i:]
        else:
            if nodes[i].type == GROUP:   # Recurse into groups
                value = type_bodies(nodes[i].value)
                nodes[i] = Node(GROUP, value, None, nodes[i].quant)
            newnodes.append(nodes[i])
            i += 1
    nodes[:] = newnodes
    return nodes

def nest_defines(nodes):
    "Attach groups to named patterns"
    newnodes = []
    i = 0
    while 1:
        if i >= len(nodes): break
        node = nodes[i]
        newnodes.append(node)
        if node.type == DEFINE:
            group = []
            while (i+1) < len(nodes) and nodes[i+1].type <> DEFINE:
                group.append(nodes[i+1])
                i += 1
            node.name = node.value
            node.value = Node(GROUP, group)
        i += 1
    nodes[:] = newnodes
    return nodes

def intersperse(nodes):
    "Look for interleaved, choice, or sequential nodes in groups/bodies"
    for node in nodes:
        if node.type in (ELEM, ATTR, GROUP, LITERAL):
            val = node.value
            ntypes = [n.type for n in val if not isinstance(val,str)]
            inters = [t for t in ntypes if t in (INTERLEAVE,CHOICE,SEQ)]
            inters = dict(zip(inters,[0]*len(inters)))
            if len(inters) > 1:
                raise ParseError, "Ambiguity in sequencing: %s" % node
            if len(inters) > 0:
                intertype = inters.keys()[0]
                items = []
                for pat in node.value:
                    if pat.type <> intertype:
                        items.append(pat)
                node.value = Node(intertype, items)
        if not isinstance(node.value, str): # No recurse to terminal str
            intersperse(node.value)
    return nodes

def scan_NS(nodes):
    "Look for any namespace configuration lines"
    global DEFAULT_NAMESPACE, OTHER_NAMESPACE, CONTEXT_FREE
    for node in nodes:
        if node.type == DEFAULT_NS:
            DEFAULT_NAMESPACE = node.value
        elif node.type == NS:
            ns, url = map(str.strip, node.value.split('='))
            OTHER_NAMESPACE[ns] = url
        elif node.type == ANNOTATION and not OTHER_NAMESPACE.has_key('a'):
            OTHER_NAMESPACE['a'] =\
              '"http://relaxng.org/ns/compatibility/annotations/1.0"'
        elif node.type == DATATYPES:
            DATATYPE_LIB[:] = [1, node.value]
        elif node.type == START:
            CONTEXT_FREE = 1

def make_nodetree(tokens):
    nodes = toNodes(tokens)
    match_pairs(nodes)
    type_bodies(nodes)
    nest_defines(nodes)
    intersperse(nodes)
    scan_NS(nodes)
    root = Node(ROOT, nodes)
    return root

if __name__=='__main__':
    make_nodetree(token_list(sys.stdin.read())).prettyprint()

