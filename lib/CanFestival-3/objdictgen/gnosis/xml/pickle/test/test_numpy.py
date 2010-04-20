
import gnosis.xml.pickle as xml_pickle
import Numeric,array
import funcs

funcs.set_parser()

class foo: pass

f = foo()

f.a = Numeric.array([[1,2,3,4],[5,6,7,8]])
f.b = Numeric.array([1.2,2.3,3.4,4.5])
f.y = array.array('b',[1,2,3,4])
f.z = array.array('f',[1,2,3,4])

a = Numeric.array([6,7,8,9])

def testfoo(o1,o2):
    for attr in ['a','b','y','z',
                 'l','d','e']:
        if getattr(o1,attr) != getattr(o2,attr):
            raise "ERROR(1)"
        
# make sure refs work
f.l = [a,a,a]

f.d = {'One':a,'Two':Numeric.array([10,11,12])}
f.e = f.d['Two']

#print f.a, f.b, f.y,f.z,f.l,f.d,f.e

x = xml_pickle.dumps(f)
#print x

#del f

g = xml_pickle.loads(x)
#print g.a,g.b,g.y,g.z,g.l,g.d,g.e

#print "XML ", x

testfoo(f,g)

print "** OK **"
