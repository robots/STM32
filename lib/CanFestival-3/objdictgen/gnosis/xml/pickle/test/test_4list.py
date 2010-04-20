
"exercise all 4 list-writing methods --fpm"

import gnosis.xml.pickle as xml_pickle
import sys
import funcs

funcs.set_parser()      
    
class foo: pass

f = foo()
f.a = (1,2,3)

# method 1 -- StreamWriter is an uncompressed StringIO
x = xml_pickle.dumps(f)

# check header (to ensure correct method used) + contents
if x[0:5] == '<?xml':
    print "OK"
else:
    print "ERROR"
    sys.exit(1)

g = xml_pickle.loads(x)
if g.a == (1,2,3):
    print "OK"
else:
    print "ERROR"
    sys.exit(1)
    
# method 2 -- StreamWriter is a compressed StringIO
x = xml_pickle.dumps(f,1)

# check header + contents
if x[0:2] == '\037\213':
    print "OK"
else:
    print "ERROR"
    sys.exit(1) 

g = xml_pickle.loads(x)
if g.a == (1,2,3):
    print "OK"
else:
    print "ERROR"
    sys.exit(1)
    
# method 3 -- StreamWriter is an uncompressed file
fh = open('aaa','wb')
xml_pickle.dump(f,fh)
fh.close()

# check header + contents
fh = open('aaa','rb')
line = fh.read(5)
if line == '<?xml':
    print "OK"
else:
    print "ERROR"
    sys.exit(1)

fh.close()

fh = open('aaa','rb')
g = xml_pickle.load(fh)
if g.a == (1,2,3):
    print "OK"
else:
    print "ERROR"
    sys.exit(1)
fh.close()

# method 4 -- StreamWriter is a compressed file
fh = open('aaa','wb')
xml_pickle.dump(f,fh,1)
fh.close()

# check header + contents
fh = open('aaa','rb')
line = fh.read(2)
if line == '\037\213':
    print "OK"
else:
    print "ERROR"
    sys.exit(1) 
    
fh.close()

fh = open('aaa','rb')
g = xml_pickle.load(fh)
if g.a == (1,2,3):
    print "OK"
else:
    print "ERROR"
    sys.exit(1)
fh.close()

funcs.unlink('aaa')
