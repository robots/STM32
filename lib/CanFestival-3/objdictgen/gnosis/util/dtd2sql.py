#!/usr/bin/python

"""Convert an XML DTD to a set of SQL 'CREATE TABLE' statements

Note:

  See http://gnosis.cx/publish/programming/xml_matters_12.txt
  for a detailed discussion of this module.

Functions:

   dict2Sql(dtd_dict)
   parseDTD(raw_dtd):

"""
__shell_usage__ = """
Shell Usage: [python] dtd2sql.py [DTD] [< DTD_from_STDIN]
"""
__version__ = "$Revision: 0.10 $"
__author__=["David Mertz (mertz@gnosis.cx)",]
__thanks_to__=[]
__copyright__="""
    This file is released to the public domain.  I (dqm) would
    appreciate it if you choose to keep derived works under terms
    that promote freedom, but obviously am giving up any rights
    to compel such.
"""

__history__="""
    0.10   Initial version.

"""
# Constants
MYSQL     = 1
DB2       = 2
ORACLE    = 3
MSSQL     = 4
POSTGRES  = 5

import string, sys, re

def dict2Sql(dtd_dict):
    create_table_lines = []
    for key, col_list in dtd_dict.items():
        stmt  = "CREATE TABLE "+key
        stmt += " (primary_key BIGINT UNSIGNED PRIMARY KEY,seq INT UNSIGNED"
        for col_name in col_list:
            if col_name[:12] == 'foreign_key_':  # foreign keys are type BIGINT
                stmt += ","+col_name+" BIGINT UNSIGNED"
            else:
                stmt += ","+col_name+" BLOB"    # other columns are BLOBs
        stmt +=  ")"
        create_table_lines.append(stmt)
    return create_table_lines

def parseDTD(raw_dtd):
    dtd_dict = {}
    for elem_def in re.findall(r"(?s)(?:<!ELEMENT\s+)(.+?)(?:\s*>)", raw_dtd):
        # split subelement disjunctions, quantifiers, and the like
        dtd_punct = ('|','(',')',',','+','*','?')
        norm_def = elem_def
        for c in dtd_punct:
            norm_def = norm_def.replace(c, " %s " % c)
        elem_split = string.split(norm_def)
        elem_name = elem_split[0].replace('-','__')
        dtd_dict[elem_name] = dtd_dict.get(elem_name,[])
        if elem_split[1] == "EMPTY":        # No subelements or PCDATA
            pass
        elif elem_split[2] == "#PCDATA":    # Has some PCDATA
            dtd_dict[elem_name].append('PCDATA')
            if elem_split[3] == "|":        # Also has subelements
                dtd_dict[elem_name].append('_XML')
                for subtag in elem_split[4:]:
                    if subtag not in dtd_punct:
                        safetag = subtag.replace('-','__')
                        if dtd_dict.has_key(safetag):
                            dtd_dict[safetag].append('foreign_key_'+elem_name)
                        else:
                            dtd_dict[safetag] = ['foreign_key_'+elem_name]
        else:                               # Has only subelements
            for subtag in elem_split[2:]:
                if subtag not in dtd_punct:
                    safetag = subtag.replace('-','__')
                    if dtd_dict.has_key(safetag):
                        dtd_dict[safetag].append('foreign_key_'+elem_name)
                    else:
                        dtd_dict[safetag] = ['foreign_key_'+elem_name]


    for att_def in re.findall(r"(?s)(?:<!ATTLIST\s+)(.+?)(?:\s*>)", raw_dtd):
        # let's make sure to isolate enumerated attribute parens
        norm_def = att_def
        for c in dtd_punct:
            norm_def = norm_def.replace(c, " %s " % c)
        att_split = string.split(norm_def)
        elem_name = att_split[0].replace('-','__')
        get_col_name = 1
        eat_quotes = 0
        for symbol in att_split[1:]:
            if get_col_name:
                dtd_dict[elem_name].append(symbol)
                get_col_name = 0        # Got a column name, eat next stuff
            elif eat_quotes:
                eat_quotes -= 1         # Decrement quote-eater
                if not eat_quotes:      # If quotes exhausted, ready for column
                    get_col_name = 1
            elif symbol in ("#IMPLIED","#REQUIRED"):
                get_col_name = 1        # Next symbol will be column name
            elif symbol == "#FIXED":
                eat_quotes = 2          # Must exhaust quoted fixed attribute

    return dtd_dict


#-- Command line version of tool
if __name__ == '__main__':
    if len(sys.argv) > 1:
        if sys.argv[1] in ('-h','/h','-?','/?','?','--help'):
            print __shell_usage__
        else:
            raw_dtd = open(sys.argv[1]).read()
            for createLine in dict2Sql(parseDTD(raw_dtd)):
                print createLine+';'
    else:
        raw_dtd = sys.stdin.read()
        for createLine in dict2Sql(parseDTD(raw_dtd)):
            print createLine+';'

