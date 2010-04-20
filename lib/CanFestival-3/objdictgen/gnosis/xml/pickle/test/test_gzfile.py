
# test binary pickling

import gnosis.xml.pickle as xml_pickle
import gzip
from StringIO import StringIO

# --- Make an object to play with ---
class C: pass
o = C()
o.lst, o.dct = [1,2], {'spam':'eggs'}

x = xml_pickle.dumps(o,1)

# make sure xml_pickle really gzipped it
sio = StringIO(x)
gz = gzip.GzipFile('dummy','rb',9,sio)
if gz.read(5) != '<?xml':
    raise "ERROR(1)"

# reload object
o2 = xml_pickle.loads(x)

# check it
if o.lst != o2.lst or o.dct != o2.dct:
    raise "ERROR(2)"

print "** OK **"

