from sys import version
from gnosis.util.introspect import data2attr, attr2data

if version >= '2.2':
    class NewList(list): pass
    class NewTuple(tuple): pass
    class NewDict(dict): pass

    nl = NewList([1,2,3])
    nt = NewTuple([1,2,3])
    nd = NewDict({1:2,3:4})
    nl.attr = 'spam'
    nt.attr = 'spam'
    nd.attr = 'spam'

    nl = data2attr(nl)
    print nl, getattr(nl, '__coredata__', 'No __coredata__')
    nl = attr2data(nl)
    print nl, getattr(nl, '__coredata__', 'No __coredata__')

    nt = data2attr(nt)
    print nt, getattr(nt, '__coredata__', 'No __coredata__')
    nt = attr2data(nt)
    print nt, getattr(nt, '__coreData__', 'No __coreData__')

    nd = data2attr(nd)
    print nd, getattr(nd, '__coredata__', 'No __coredata__')
    nd = attr2data(nd)
    print nd, getattr(nd, '__coredata__', 'No __coredata__')
else:
    print "data2attr() and attr2data() only work on 2.2+ new-style objects"


