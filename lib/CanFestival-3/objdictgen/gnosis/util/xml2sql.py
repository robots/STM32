#!/usr/bin/python

"""Convert an XML document to a set of SQL 'INSERT INTO' statements

Note:

  See http://gnosis.cx/publish/programming/xml_matters_12.txt
  for a detailed discussion of this module.

Functions:

  walkNodes(py_obj)

"""
__shell_usage__ = """
Shell Usage: [python] xml2sql.py [XML] [< XML_from_STDIN]
"""
__version__ = "$Revision: 0.10 $"
__author__=["David Mertz (mertz@gnosis.cx)",]
__thanks_to__=["Tom Lynn (tom@fish.cx)",]
__copyright__="""
    This file is released to the public domain.  I (dqm) would
    appreciate it if you choose to keep derived works under terms
    that promote freedom, but obviously am giving up any rights
    to compel such.
"""

__history__="""
0.10    Initial version.

Apr 04  Tom Lynn correction where child element and attribute
        have same name
"""
# Constants
MYSQL     = 1
DB2       = 2
ORACLE    = 3
MSSQL     = 4
POSTGRES  = 5
MAX_PCDATA = "10000"
MAX_XML    = "10000"

# Imports
import string, sys, re, random
from time import time
from types import *
from gnosis.xml.objectify import XML_Objectify, DOM, EXPAT, _XO_
from gnosis.xml.objectify import keep_containers, MAYBE

# Module setup
insertLines = []
random.seed(time())

def walkNodes(py_obj, parent_info=('',''), seq=0):
    insDct = {}     # initialize the INSERT statement dictionary
                    # table name must be unmangled from [xml_objectify] name
    insDct['table_name'] = py_obj.__class__.__name__.replace('_XO_','')
    insDct['table_name'] = insDct['table_name'].replace('-','__')
                    # column name of parent massaged w/ 'foreign_key_' prefix
    insDct['parent_name'] = 'foreign_key_' + parent_info[0]
                    # primary key of parent table
    insDct['parent_key'] = parent_info[1]
                    # primary key is stringified random BIGINT
    insDct['primary_key'] = str(long(random.random() * 10**18L))
                    # start out with empty strings of other columns/values
    insDct['other_cols'] = ""
    insDct['other_vals'] = ""
    insDct['seq'] = str(seq)

    self_info = (insDct['table_name'],insDct['primary_key'])

    for colname in py_obj.__dict__.keys():
        safename = colname.replace('-','__')
        if colname == "__parent__":
           continue             # ExpatFactory uses bookeeping attribute

        member = getattr(py_obj,colname)
        if type(member) == InstanceType:
            walkNodes(member, self_info)
        elif type(member) == ListType:
            for memitem in member:
                if isinstance(memitem,_XO_):
                    seq += 1
                    walkNodes(memitem, self_info, seq)
        else:
            member = member.replace('"',r'\"')
            member = member.replace('\n',r'\n')
            member = member.replace('\r',r'\r')
            member = member.replace('\t',r'\t')
            insDct['other_cols'] += ','+safename
            insDct['other_vals'] += ',"'+member+'"'

    if parent_info[0]:      # non-root node
        stmt  = "INSERT INTO %(table_name)s "                    \
                         "(primary_key,seq,%(parent_name)s"      \
                                           "%(other_cols)s) "    \
                "VALUES (%(primary_key)s,%(seq)s,%(parent_key)s" \
                                           "%(other_vals)s)"     % insDct
        insertLines.append(stmt)
    else:                   # the root node
        stmt  = "INSERT INTO %(table_name)s "                    \
                    "(primary_key) VALUES (%(primary_key)s)"     % insDct
        insertLines.append(stmt)


#-- Command line version of tool
if __name__ == '__main__':
    if len(sys.argv) > 1:
        if sys.argv[1] in ('-h','/h','-?','/?','?','--help'):
            print __shell_usage__
        else:
            keep_containers(MAYBE)      # Keep XML iff character markup
            py_obj = XML_Objectify(sys.argv[1],DOM).make_instance()
            walkNodes(py_obj)
            for insertLine in insertLines:
                print insertLine.encode('UTF-8')+';'
    else:
        keep_containers(MAYBE)      # Keep XML iff character markup
        py_obj = XML_Objectify(sys.stdin,DOM).make_instance()
        walkNodes(py_obj)
        for insertLine in insertLines:
            print insertLine.encode('UTF-8')+';'


