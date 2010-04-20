"""Store Python objects to (pickle-like) XML Documents

Please see the information at gnosis.xml.pickle.doc for
explanation of usage, design, license, and other details
"""
from gnosis.xml.pickle._pickle import \
     XML_Pickler, XMLPicklingError, XMLUnpicklingError, \
     dump, dumps, load, loads

from gnosis.xml.pickle.util import \
     setParanoia, getParanoia, setDeepCopy, getDeepCopy,\
     get_class_from_store, add_class_to_store, remove_class_from_store,\
     setParser, setVerbose, enumParsers

from gnosis.xml.pickle.ext import *
