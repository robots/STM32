""" using getstate() to rename attributes into a more verbose
form for XML output (also tests setstate)  --fpm"""

class contact:
    def __init__(self,first,last,addr,city,state):
        self.first = first
        self.last = last
        self.addr = addr
        self.city = city
        self.state = state

    def __getstate__(self):
        d = {'FirstName':self.first,
             'LastName':self.last,
             'Address':self.addr,
             'City':self.city,
             'State':self.state}
        return d

    def __setstate__(self,d):
        #print "GOT __setstate__!"
        self.first = d['FirstName']
        self.last = d['LastName']
        self.addr = d['Address']
        self.city = d['City']
        self.state = d['State']

    #def hi(self):
    #	 print self.first,self.last,self.addr,self.city,self.state

import gnosis.xml.pickle as xml_pickle
from gnosis.xml.pickle.util import setParanoia
import funcs

funcs.set_parser()

setParanoia(0)

def checkit(o1,o2):
    for attr in ['first','last','addr','city','state']:
        if getattr(o1,attr) != getattr(o2,attr):
            raise "ERROR(1)"
        
c = contact('Joe','Jones','1744 Elk Road','Manchester','NH')
#c.hi()

x = xml_pickle.dumps(c)
#print x
#del c

d = xml_pickle.loads(x)
#d.hi()
checkit(c,d)

# just to show this is a legal pickleable object ...
import pickle

s = pickle.dumps(d)
#del d

q = pickle.loads(s)
#q.hi()

checkit(c,q)

print "** OK **"
