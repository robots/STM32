#!/usr/bin/python

"""Convert an SQL query to a DTD that specifies an XML
dialect for query results

Note:

  See http://gnosis.cx/publish/programming/xml_matters_9.txt
  for a detailed discussion of this module and its companion,
  Scott Hathaway's [sql2xml].

  [sql2xml] and its support [db] module can be downloaded from
  http://members.home.net/slhath/downloads.html.

Functions:

    parseSQL(query)
    parsedQuery2DTD(parseStruct,query="")
    parsedQuery2Fields(parseStruct)
    typifyDTD(dtd, column_type_dict)

Module Usage:

  Usage by other modules is straghtforward.  First you want to
  produce a parsed component structure of an SQL statement, then
  you want to use the parsed component list to generate an output
  format (usually a DTD):

    import sql2dtd
    sql_query = "SELECT ..."
    parsedSQL = sql2dtd.parseSQL(query)
    dtd = sql2dtd.parsedQuery2DTD(parsedSQL,query)
    # ...do something with the DTD...

"""
__shell_usage__ = """
Shell Usage: [python] sql2dtd.py [SQL_query] [< SQL_from_STDIN]
"""
__version__ = "$Revision: 0.10 $"
__author__=["David Mertz (mertz@gnosis.cx)",]
__thanks_to__=["Scott Hathaway (hathaways@infotrain.com)",]
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

import string,sys

def parseSQL(query):
    "Return a structure of parsed components of an SQL query"

    # Known issues:
    #  * Does not handle UNION'd clauses.
    #  * Computed columns use generic position name since neither
    #    the SQL function or its arguments is necessarily unique
    #    between columns.

    # Normalize the query slightly
    query = string.join(string.split(query))    # One space between words
    query = string.replace(query,", ",",")      # Comma'd items together
    query = string.replace(query," ,",",")      # Comma'd items together
    query = string.replace(query,"( ","(")      # No space after left paren
    query = string.replace(query," )",")")      # No space before right paren
    query = string.replace(query," as ","!")    # Special handling of AS
    query = string.replace(query," AS ","!")    # Special handling of AS
    query = string.replace(query," As ","!")    # Special handling of AS
    sql_terms = string.split(query)

    # Initial validity test
    if string.upper(sql_terms[0]) <> 'SELECT':
        raise ValueError, "SELECT query must be specified"

    # Go to it...
    ignore_adjectives = ('STRAIGHT_JOIN','SQL_SMALL_RESULT','DISTINCT','ALL')
    column_names = []
    group_bys = ""
    got_columns = 0
    for ndx in range(1,len(sql_terms)):
        term = sql_terms[ndx]
        if term in ignore_adjectives:
            pass
        elif not got_columns:
            column_names = string.split(term,",")
            got_columns = 1
        elif (string.upper(term)=='GROUP' and
              string.upper(sql_terms[ndx+1])=='BY'):
            group_bys = string.join(string.split(sql_terms[ndx+2],","))

    # Use the AS version of row name, if given
    for i in range(len(column_names)):
        column_names[i] = string.split(column_names[i],'!')[-1]

    # Massage computed row names ...use generic name 'columnXX',
    # but store actual computation to column_attr dict
    column_attr = {}
    for i in range(len(column_names)):
        column_name = column_names[i]
        if "(" in column_name:
            seq_name = 'column'+`i+1`
            column_attr[seq_name] = column_name
            column_names[i] = seq_name

    return column_names, group_bys, column_attr

def parsedQuery2DTD(parseStruct,query=""):
    "Convert a parsed SQL structure (and a raw query) into a matching DTD"

    column_names = parseStruct[0]
    group_bys    = parseStruct[1]
    column_attr  = parseStruct[2]
    dtd_lst = []

    #-- Root (SQL) element and attributes (if any)
    dtd_lst.append("<!ELEMENT SQL (row)*>")
    dtd_lst.append("<!ATTLIST SQL")
    if group_bys:
        dtd_lst.append('  GROUP_BY NMTOKEN #FIXED "%s"' % group_bys)
    if query:
        dtd_lst.append('  query CDATA #FIXED "%s"' % query)
    dtd_lst.append(">") # close the root ATTLIST

    #-- <row> element and attribute 'num'
    columns_str = string.replace(`tuple(column_names)`,"'","")
    dtd_lst.append("<!ELEMENT row %s>" % columns_str)
    dtd_lst.append("<!ATTLIST row num ID #IMPLIED>")

    #-- Individual columns (with attribute if calculated)
    for column in column_names:
        dtd_lst.append("<!ELEMENT %s (#PCDATA)>" % column)
        if column_attr.has_key(column):
            dtd_lst.append('<!ATTLIST %s CALC CDATA #FIXED "%s">' %
                                      (column, column_attr[column]))
    return string.join(dtd_lst,'\n')

def parsedQuery2Fields(parseStruct):
    "Convert a parsed SQL structure to sql2xml's pipe-delimited column list"
    column_names = parseStruct[0]
    return string.join(column_names,'|')

def typifyDTD(dtd, column_type_dict):
    "Add column type information to a DTD"
    # not yet implemented
    return dtd

def textResults2XML(table, column_names, RDBMS=MYSQL):
    "Transform text-form query results to XML markup"
    # not yet implemented (meaningfully)
    xml = ['<SQL>']
    row = 0
    for line in string.split(table,'\n'):
        row = row+1
        xml.append('  <row num=%s>' % row)
        xml.append('    %s' % line)
        xml.append('  </row>')
    xml.append('</SQL>')
    return string.join(xml,'\n')


#-- Command line version of tool
if __name__ == '__main__':
    if len(sys.argv) > 1:
        if sys.argv[1] in ('-h','/h','-?','/?','?','--help'):
            print __shell_usage__
        else:
            query = sys.argv[1]
            print parsedQuery2DTD(parseSQL(query),query)
            # print parsedQuery2Fields(parseSQL(query))
    else:
        query = sys.stdin.read()
        print parsedQuery2DTD(parseSQL(query),query)
