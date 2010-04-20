
"""
each test_*.py imports this to auto-set the parser based on
the presence of a USE_... file

-- frankm@hiwaay.net
"""

import os, sys, string
import gnosis.xml.pickle as xml_pickle

# set parser based on USE_... file present
def set_parser():
    if os.path.isfile('USE_SAX'):
        xml_pickle.setParser("SAX")
    elif os.path.isfile('USE_CEXPAT'):
        xml_pickle.setParser('cEXPAT')
    else:
        xml_pickle.setParser('DOM')

# cheap substitutes for some shell functions
def unlink(filename):
    if not os.path.isfile(filename):
        return

    # eegh ... convoluted, but this is the only
    # way I made it work for both Linux & Win32 
    try: os.unlink(filename)
    except: pass

    try: os.remove(filename)
    except: pass

def touch(filename):
    open(filename,'w')

#def pyver():
#    return string.split(sys.version)[0] 
