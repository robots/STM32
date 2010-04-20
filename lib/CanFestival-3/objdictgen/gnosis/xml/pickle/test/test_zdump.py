from zlib import compress
import cPickle
import gnosis.xml.pickle as xml_pickle

class C: pass

#-----------------------------------------------------------------------
# First: look at the results of compression on a large object
#-----------------------------------------------------------------------
o = C()
from os import sep
fname = sep.join(xml_pickle.__file__.split(sep)[:-1])+sep+'_pickle.py'
o.lst = open(fname).readlines()
o.tup = tuple(o.lst)
o.dct = {}
for i in range(500):
    o.dct[i] = i

print "Size of standard cPickle:    ", len(cPickle.dumps(o))
print "Size of binary cPickle:      ", len(cPickle.dumps(o,1))

print "gzip'd standard cPickle:     ", len(compress(cPickle.dumps(o)))
print "gzip'd binary cPickle:       ", len(compress(cPickle.dumps(o,1)))

print "Size of standard xml_pickle: ", len(xml_pickle.dumps(o))
print "Size of gzip'd xml_pickle:   ", len(xml_pickle.dumps(o,1))

#-----------------------------------------------------------------------
# Second: look at actual compressed and uncompressed (small) pickles
#-----------------------------------------------------------------------
o = C()
o.lst = [1,2]
o.dct = {'spam':'eggs'}

print "--------------- Uncompressed xml_pickle: ---------------"
print xml_pickle.dumps(o),
print "--------------- Compressed xml_pickle: -----------------"
print `xml_pickle.dumps(o,1)`

#-----------------------------------------------------------------------
# Third: make sure compressed pickle makes it through restore cycle
#-----------------------------------------------------------------------
print "------------------------------------------------------"
try:
    xml_pickle.loads(xml_pickle.dumps(o))
    print "Pickle/restore cycle with compression:  OK"
except:
    print "Pickle/restore cycle with compression:  FAILED!"

try:
    xml_pickle.loads('CORRUPT'+xml_pickle.dumps(o))
    print "Pickle/restore corrupt data with compression:  OK"
except:
    print "Pickle/restore corrupt data with compression:  FAILED!"

