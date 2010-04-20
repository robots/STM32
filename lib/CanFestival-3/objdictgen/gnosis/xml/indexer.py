#!/usr/bin/env python

"""Create full-text XPATH indexes of XML documents

Notes:

  See http://gnosis.cx/publish/programming/xml_matters_10.html
  for a detailed discussion of this module.

  To make this module work, you will want the following:

    [indexer] module:
      http://gnosis.cx/download/indexer.py

    [xml_objectify] module:
      http://gnosis.cx/download/xml_objectify.py

Classes:

  XML_Indexer(PreferredIndexer, TextSplitter)

"""
__shell_usage__ = """
Shell Usage: [python] xml_indexer.py [options] [xml_file [xml_file2 [...]]]

    -h, /h, -?, /?, ?, --help:    Show this help screen
"""
__version__ = "Mon 02-24-2003"
__author__=["David Mertz (mertz@gnosis.cx)",]
__thanks_to__=[]
__copyright__="""
    This file is released to the public domain.  I (dqm) would
    appreciate it if you choose to keep derived works under terms
    that promote freedom, but obviously am giving up any rights
    to compel such.
"""
__history__="""
  0.05  Pre-initial "proof-of-concept"

  0.10  Initial version.

  0.20  Module refactored into gnosis.xml package.  This is a
        first pass, and various documentation and test cases
        should be added later.

  02/03 Fixed some errors in the transition to Gnosis Utilities
        packaging.
"""
from types import *

# Names from xml_objectify (and configure xml_objectify)
from gnosis.xml.objectify import XML_Objectify, keep_containers
from gnosis.xml.objectify import ALWAYS, MAYBE, NEVER, DOM, EXPAT
keep_containers(MAYBE)

# Which concrete indexer should serve as the base for XML_Indexer?
import gnosis.indexer as indexer

# Create a class to implement XPATH-like indexing
class XML_Indexer(indexer.PreferredIndexer, indexer.TextSplitter):
    """Concrete Indexer for XML-as-hierarchical-filesystem
    """
    def add_file(self, fname):
        "Index the nodes of an XML file"
        # Read in the file (if possible)
        try:
            py_obj = XML_Objectify(fname, EXPAT).make_instance()
            if not py_obj:      # Fallback to DOM where Expat has problems
                raise "BadPaserError"
                py_obj = XML_Objectify(fname, DOM).make_instance()
            if self.quiet < 5: print "Indexing", fname
        except IOError:
            return 0
        self.fname_prefix = fname
        self.recurse_nodes(py_obj)

    def recurse_nodes(self, currnode, xpath_suffix=""):
        "Recurse and process nodes in XML file"
        if hasattr(currnode, '_XML'):   # maybe present literal XML of object
            text = currnode._XML.encode('UTF-8')
            self.add_nodetext(text, xpath_suffix)
        elif not isinstance(currnode, unicode):	       # Uche fix
            for membname in currnode.__dict__.keys():
                if membname in ("__parent__","_seq"):  # ditto, Uche
                   continue     # ExpatFactory uses bookeeping attributes
                member = getattr(currnode, membname)
                if type(member) is InstanceType:
                    xpath = xpath_suffix+'/'+membname
                    self.recurse_nodes(member, xpath.encode('UTF-8'))
                elif type(member) is ListType:
                    for i in range(len(member)):
                        xpath = xpath_suffix+'/'+membname+'['+str(i+1)+']'
                        self.recurse_nodes(member[i], xpath.encode('UTF-8'))
                elif type(member) is StringType:
                    if membname != 'PCDATA':
                        xpath = xpath_suffix+'/@'+membname
                        self.add_nodetext(member, xpath.encode('UTF-8'))
                    else:
                        self.add_nodetext(member, xpath_suffix.encode('UTF-8'))
                elif type(member) is UnicodeType:
                    if membname != 'PCDATA':
                        xpath = xpath_suffix+'/@'+membname
                        self.add_nodetext(member.encode('UTF-8'),
                                          xpath.encode('UTF-8'))
                    else:
                        self.add_nodetext(member.encode('UTF-8'),
                                          xpath_suffix.encode('UTF-8'))
                else:
                    raise TypeError, \
                          "Unsupported Node Type: "+`type(member)`+`member`

    def add_nodetext(self, text, xpath_suffix):
        "Add the node PCDATA to index, using full XPATH to node as key"
        node_id = self.fname_prefix+'::'+xpath_suffix
        words = self.splitter(text, 'text/plain')

        # Find new node index, and assign it to node_id
        # (_TOP uses trick of negative to avoid conflict with file index)
        self.files['_TOP'] = (self.files['_TOP'][0]-1, None)
        node_index =  abs(self.files['_TOP'][0])
        self.files[node_id] = (node_index, len(words))
        self.fileids[node_index] = node_id

        for word in words:
            if self.words.has_key(word):
                entry = self.words[word]
            else:
                entry = {}
            if entry.has_key(node_index):
                entry[node_index] = entry[node_index]+1
            else:
                entry[node_index] = 1
            self.words[word] = entry

#-- If called from command-line, parse arguments and take actions
if __name__ == '__main__':
    import os,sys
    if len(sys.argv)>=2:
        if sys.argv[1] in ('-h','/h','-?','/?','?','--help'):   # help screen
            print __shell_usage__
        else:
            ndx = XML_Indexer()
            ndx.load_index()
            for file in sys.argv[1:]:
                ndx.add_file(file)
            ndx.save_index()
    else:
        sys.stderr.write("Perhaps you would like to use the --help option?\n")
