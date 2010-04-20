"""
Examples of several different things mutators can do. --fpm
"""

import gnosis.xml.pickle as xml_pickle
import gnosis.xml.pickle.ext as mutate
import gnosis.xml.pickle.util as util
from gnosis.xml.pickle.ext import XMLP_Mutator, XMLP_Mutated
from types import *
import sys
from UserList import UserList
import funcs

funcs.set_parser()
    
class mystring(XMLP_Mutator):

    def mutate(self,obj):
        print "** mystring.mutate()"
        return XMLP_Mutated(obj)

    def unmutate(self,mobj):
        print "** mystring.unmutate()"
        return mobj.obj
    
# test1 -- use our custom handler to pickle & unpickle
# (here we fold two types to a single tagname)

print "*** TEST 1 ***"
my1 = mystring(StringType,"MyString",in_body=1)
my2 = mystring(UnicodeType,"MyString",in_body=1)

mutate.add_mutator(my1)
mutate.add_mutator(my2)

u = UserList(['aaa','bbb','ccc'])
print u

x = xml_pickle.dumps(u)
print x
del u

z = xml_pickle.loads(x)
print z

# remove custom mutators
mutate.remove_mutator(my1)
mutate.remove_mutator(my2)

#
# test 2 -- custom pickler, but builtin unpickler
#

# use same tagname as builtin so builtin can unpickle it
# (put text in body, even though builtin puts it in
# the value= ... just to show that builtin can find it either way)

print "*** TEST 2 ***"

my1 = mystring(StringType,"string",in_body=1)
my2 = mystring(UnicodeType,"string",in_body=1)

mutate.add_mutator(my1)
mutate.add_mutator(my2)

u = UserList(['aaa','bbb','ccc'])
print u

x = xml_pickle.dumps(u)
print x
del u

# remove custom mutators before unpickling
mutate.remove_mutator(my1)
mutate.remove_mutator(my2)

# now the builtin is doing the unpickling
z = xml_pickle.loads(x)
print z

# test 3
#
# show how mutators can be chained together for neat purposes
# (though this example is pretty useless :-)
#
# mynumlist handles lists of integers and pickles them as "n,n,n,n"
# mycharlist does the same for single-char strings
#
# otherwise, the ListType builtin handles the list

class mynumlist(XMLP_Mutator):

    def wants_obj(self,obj):
        # I only want lists of integers
        for i in obj:
            if type(i) is not IntType:
                return 0

        return 1
    
    def mutate(self,obj):
        t = '%d'%obj[0]
        for i in obj[1:]:
            t = t + ',%d'%i
        return XMLP_Mutated(t)

    def unmutate(self,mobj):
        l = map(int,mobj.obj.split(','))
        return l

class mycharlist(XMLP_Mutator):

    def wants_obj(self,obj):
        # I only want lists of single chars
        for i in obj:
            if type(i) is not StringType or \
               len(i) != 1:
                return 0

        return 1

    def mutate(self,obj):
        t = '%s'%obj[0]
        for i in obj[1:]:
            t = t + ',%s'%i
        return XMLP_Mutated(t)

    def unmutate(self,mobj):
        l = mobj.obj.split(',')
        return l

# unlike test#1 (folding multiple types -> one tag), here
# we "explode" one type to multiple tags, so we can
# make different XML representations based on object content

print "*** TEST 3 ***"

my1 = mynumlist(ListType,"NumList",in_body=1)
my2 = mycharlist(ListType,"CharList",in_body=1)

mutate.add_mutator(my1)
mutate.add_mutator(my2)

u = UserList([[1,2,3],['mmm','nnn','ooo'],['a','b','c'],[4.1,5.2,6.3]])
print u
x = xml_pickle.dumps(u)
print x
del u

g = xml_pickle.loads(x)
print g

# remove custom mutators
mutate.remove_mutator(my1)
mutate.remove_mutator(my2)

#
# test 4
#
# A mutator that takes UserList-derived classes and saves them
# as 'seq' types instead of 'PyObject' (the default) (i.e. a parser
# that understands only 'seq' could now read an XML file containing
# UserList-derived classes)
# 

# note: being a simple example, this doesn't take into account
#		things like initargs that would be needed to make this
#		a complete implementation

# note 2: this is basically how we handle "newstyle" objects in
#		  Python 2.2+, coincidentally enough :-) 

class mutate_userlist(XMLP_Mutator):
    def __init__(self):
        XMLP_Mutator.__init__(self,type(UserList()),'userlist')

    # type(UserList()) should be InstanceType, which is
    # pretty generic, so we have to be careful and check
    # that the obj is a UserList (or derived)
    def wants_obj(self,obj):
        return isinstance(obj,UserList)

    # OTOH, we want all mutated objects that match our tag
    # ('userlist'), so do nothing for 'wants_mutated()'

    def mutate(self,obj):
        return XMLP_Mutated(obj.data,
                            "%s.%s"%(util._module(obj),util._klass(obj)))

    def unmutate(self,mobj):
        p = mobj.extra.split('.')
        klass = util.get_class_from_name(p[1],p[0],
                                         xml_pickle.getParanoia())		
        return klass(mobj.obj)

print "*** TEST 4 ***"

xml_pickle.setParanoia(0) # let xml_pickle use our namespace

my1 = mutate_userlist()
mutate.add_mutator(my1)

class mylist(UserList):pass
class zlist(mylist): pass

class foo:pass
f = foo()

f.u = UserList([1,2,3,4])
f.m = mylist([5,6,7,8])
f.z = zlist([9,10,11,12])

print f.u.__class__, f.u
print f.m.__class__, f.m
print f.z.__class__, f.z

x = xml_pickle.dumps(f)
print x
del f

g = xml_pickle.loads(x)
print g.u.__class__, g.u
print g.m.__class__, g.m
print g.z.__class__, g.z

mutate.remove_mutator(my1)

# skip test 5 if Numeric not installed
try:
    import Numeric
except:
    sys.exit(0)

#
# test 5 -- this mutator takes a multidimensional Numerical.array and
# pretty-prints it as a matrix in the XML body
#
# (this could be a whole lot fancier, but I kept it simple & dumb for
# demonstration purposes)
#

class mutate_multidim(XMLP_Mutator):
    def __init__(self):
        XMLP_Mutator.__init__(self,Numeric.ArrayType,'numpy_matrix',in_body=1)

    def mutate(self,obj):
        s = '\n' # start on new line in XML file
        for arr in obj:
            for elem in arr:
                s += '%f ' %elem
            s += '\n'
            
        return XMLP_Mutated(s)

    def unmutate(self,mobj):
        text = mobj.obj
        lines = text.split('\n')
        list = []
        for line in lines:
            a = map(float,line.split())
            if len(a):
                list.append(a)

        a = Numeric.array(list)
        return a

    
my1 = mutate_multidim()
mutate.add_mutator(my1)

f = foo()

f.a = Numeric.array([[1,2,3,4],[5,6,7,8],[9,10,11,12]],'f')

print "ORIG: ",f.a

x = xml_pickle.dumps(f)
print x
del f

g = xml_pickle.loads(x)
print "COPY: ",g.a

mutate.remove_mutator(my1)

#
# test 6 -- override the default mxDateTime handlers
#			the default handlers prints the date/time as a single
#			string. this mutator makes it a 2-element dict instead,
#			one for date, one for time.
#
try:
    import mx.DateTime, re
    mxDateTime_type = type(mx.DateTime.localtime())
except:
    # skip test if mxDateTime not installed
    sys.exit(0)

class mutate_mxdatetime(XMLP_Mutator):

    def __init__(self):
        XMLP_Mutator.__init__(self,mxDateTime_type,'mxDateTime',
                              paranoia=0)
        
    def mutate(self,obj):
        # save as a dict --
        #  obj['YMD'] = "year/month/day"
        #  obj['HMS'] = hour/min/second 
        
        # (I avoid using strftime(), for portability reasons.)
        # Pickle seconds as a float to save full precision.
        d = {}
        d['YMD'] = "%d/%d/%d" % (obj.year,obj.month,obj.day)
        d['HMS'] = "%d:%d:%f" % (obj.hour,obj.minute,obj.second)
        return XMLP_Mutated(d)
    
    def unmutate(self,mobj):
        obj = mobj.obj
        # is this forgiving enough? :-)
        fmt = '\s*([0-9]+)\s*/\s*([0-9]+)\s*/\s*([0-9]+)'
        m1 = re.match(fmt,obj['YMD'])
        fmt = '\s*([0-9]+)\s*:\s*([0-9]+)\s*:\s*([0-9\.]+)'
        m2 = re.match(fmt,obj['HMS'])
        # this started giving a deprecation warning about passing a
        # float where an int was expected
        # return apply(mx.DateTime.DateTime,map(float,m1.groups())+
        #	 	 map(float,m2.groups()))

        return apply(mx.DateTime.DateTime,map(int,m1.groups())+
                     map(int,m2.groups()[:2])+[float(m2.group(3))])

my1 = mutate_mxdatetime()
mutate.add_mutator(my1)

f = foo()

f.a = mx.DateTime.now()

print "ORIG: ",f.a

x = xml_pickle.dumps(f)
print x
del f

g = xml_pickle.loads(x)
print "COPY: ",g.a

mutate.remove_mutator(my1)
