import gnosis.xml.pickle as xml_pickle

class Test: pass

o1 = Test()
o1.s = "o1"

o2 = Test()
o2.s = "o2"

o1.obj1 = o2
o1.obj2 = o2
o2.obj3 = o1
o2.obj4 = o1

xml = xml_pickle.dumps(o1)
#print xml

z = xml_pickle.loads(xml)

# check it
if z.s != o1.s or z.obj1.s != o1.obj1.s or \
   z.obj2.s != o1.obj2.s or z.obj1.obj3.s != o1.obj1.obj3.s or \
   z.obj2.obj4.s != o1.obj2.obj4.s:
    raise "ERROR(1)"

print "** OK **"




