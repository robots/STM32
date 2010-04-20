#! /usr/bin/env python
#
# -*- coding: iso-8859-1 -*-
#
#
# FILE:         EDS2CSV.py
# BEGIN:        Nov 30,2007
# AUTHOR:       Giuseppe Massimo Bertani
# EMAIL         gmbertani@users.sourceforge.net
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  Coding style params:
#  <TAB> skips=4
#  <TAB> are replaced with blanks
#
#

import sys
import os
import ConfigParser as cp

if (len(sys.argv) != 2):
    print "Usage:"
    print 
    print "EDS2CSV.py <input file> "
    print
    print
    sys.exit(0)

EDSname = os.path.abspath( os.path.dirname(sys.argv[1]) ) + '/' + sys.argv[1] 

if (os.path.exists(EDSname) is not True):
    print 
    print "Input file ",EDS2CSV," not found."
    print
    print
    sys.exit(0)

eds = cp.ConfigParser()
eds.read(EDSname)
    
ssorted = sorted(eds.sections())

# dump entire EDS file to stdout in CSV format comma separated
print "Object Dictionary,",sys.argv[1]
print
for section in ssorted:
    print section
    print ",", 
    osorted = sorted(eds.options(section))
    for option in osorted:
        print option, ",", 
    print
    print ",", 
    for option in osorted:
        print eds.get(section, option), ",",    
    print
    print

