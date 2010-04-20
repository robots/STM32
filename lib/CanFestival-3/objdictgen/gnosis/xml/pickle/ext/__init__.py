"""The gnosis.xml.pickle type extension system.

Please see [..]/gnosis/xml/pickle/doc/HOWTO.extensions for details
"""
__author__ = ["Frank McIngvale (frankm@hiwaay.net)",
              "David Mertz (mertz@gnosis.cx)",
             ]

from _mutate import \
     can_mutate,mutate,can_unmutate,unmutate,\
     add_mutator,remove_mutator,XMLP_Mutator, XMLP_Mutated, \
     get_unmutator, try_mutate

