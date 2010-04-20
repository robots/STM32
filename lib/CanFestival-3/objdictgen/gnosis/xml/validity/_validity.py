"""Library of Python objects that conform to XML validity rules

Requires: Python 2.2+
(may use new-style classes, slots, properties, metaclasses)

TODO: Need ANY type for <!ELEMENT tag ANY> declarations
      (not to be confused with Any(Quantification)
"""
from operator import add
from sys import maxint

class ValidityError(TypeError): pass
class LengthError(ValidityError): pass
class ReprError(ValidityError): pass

def match_type(item, types):
    # We might use metaclass, so don't check actual 'type(item)'
    # ...instead, just be satisfied if it -looks- like right thing
    return repr(type(item)) in map(repr, types)

class PCDATA(unicode):
    def __init__(self, s):
        self._tag = self.__class__.__name__
    def __str__(self):
        if self._tag[0] == '_' or self.__class__ is PCDATA:
            return self.encode('utf-8')
        else:
            return '<%s>%s</%s>\n' % \
                   (self._tag, self.encode('utf-8'), self._tag)
    def validate(self): pass

toPCDATA = lambda lst: map(PCDATA, lst)

class EMPTY(object):
    def __init__(self, initlist=[]):
        self._tag = self.__class__.__name__
    def __str__(self):
        self.validate()
        return '<%s />' % (self._tag)
    def validate(self):
        if self._tag[0] == '_':
            raise ReprError, \
                  "Initial underscore in class %s indicates non-tag" \
                  % (self._tag,)

class Or(object):
    def __init__(self, disjunct):
        if not hasattr(self.__class__, '_disjoins'):
            raise NotImplementedError, \
                  "Child of Abstract Class Or must specify list of disjoins"
        self._tag = self.__class__.__name__
        disjunct = self.lift(disjunct)
        self.checkitem(disjunct)
        self._data = disjunct
    def __str__(self):
        if self._tag[0] == '_':
            return '%s' % self._data
        else:
            return '<%s>%s</%s>\n' % (self._tag, self._data, self._tag)
    def checkitem(self, item):
        if not match_type(item, self._disjoins):
            raise ValidityError, \
                  "\n%s not in: \n  %s" % \
                  (type(item),'\n  '.join(map(repr,self._disjoins)))
    def validate(self):
        self.checkitem(self._data)
    def lift(self, item):
        if PCDATA in self._disjoins and type(item) in (str, unicode):
            item = PCDATA(item)
        # XXX -- can we reason abt other disjoins and their initializers?
        return item


class Seq(tuple):
    """Abstract class for sequence of elements

    Inheritance from immutable type is tricky.  A mutable class like
    Quantification descendents can 'lift' initialization arguments before
    calling base type initialization.  Seq children cannot do this.
    Therefore, use of the factory-function LiftSeq is generally recommended
    for more flexible initialization of a Seq object, e.g.:

        class tag1(Some): _type = whatever
        elm1 = tag1([whatever('this'),  whateverlike])
        class tag2(Seq): _order = (this, that, other)
        elm2 = LiftSeq(tag1, (thislike, thatlike, otherlike))

    The actual list parameter to tag1() can contain 'liftable' items,
    but the same effect for Seq passes 'tag2' to LiftSeq().
    """
    def __init__(self, inittup):
        if not hasattr(self.__class__, '_order'):
            raise NotImplementedError, \
                  "Child of Abstract Class Seq must specify order"
        if not isinstance(self._order, tuple):
            raise ValidityError, "Seq must have tuple as 'order'"
        self.validate()
        self._tag = self.__class__.__name__
    def __str__(self):
        contents = ''.join([str(item) for item in self])
        if self._tag[0] == '_':
            return contents
        else:
            return '<%s>%s</%s>\n' % (self._tag, contents, self._tag)
    def validate(self):
        for item, type_ in zip(self, self._order):
            if not match_type(item,[type_]):
                raise ValidityError, \
                      "Item %s must be of type %s (in %s)" % \
                      (type(item), type_, self._tag)

def LiftSeq(klass, items):
    #-- Make sure klass has necessary _order specification
    if not hasattr(klass, '_order'):
        raise NotImplementedError, \
              "Child of Abstract Class Seq must specify order"
    #-- Examine items:
    # (1) is it the right length?
    if len(items) != len(klass._order):
        raise LengthError, \
              "Sequence for %s must contain %s elements" % \
              (klass.__name__, len(klass._order))
    # (2) is each item usable (lifted if necessary)
    lifted = []
    for item, type_ in zip(items, klass._order):
        if match_type(item,[type_]):
            lifted.append(item)
        elif issubclass(type_, Seq):
            lifted.append(LiftSeq(type_, item))
        else:
            lifted.append(type_(item))
    return klass(tuple(lifted))

class Quantification(list):
    def __init__(self, initlist=[]):
        if not hasattr(self.__class__, '_type'):
            raise NotImplementedError, \
                  "Child of Abstract Quantification must specify type_"
        self._tag = self.__class__.__name__
        initlist = self.lift(initlist)
        if not self.min_length <= len(initlist) <= self.max_length:
            raise LengthError, self.length_message % self._tag
        else:
            list.__init__(self, initlist)
    def __setitem__(self, pos, item):
        self.checkitem(item)
        list.__setitem__(self, pos, item)
    def append(self, item):
        self.extend([item])
    def extend(self, items):
        items = self.lift(items)
        if len(self)+len(items) > self.max_length:
            raise LengthError, self.length_message % self._tag
        else:
            for item in items:
                self.checkitem(item)
            list.extend(self, items)
    def __iadd__(self, items):
        self.extend(items)
        return self
    def __delitem__(self, i):
        if len(self)-1 < self.min_length:
            raise LengthError, self.length_message % self._tag
        list.__delitem__(self, i)
    def __str__(self):
        contents = ''.join([str(item) for item in self])
        if self._tag[0] == '_':
            return contents
        else:
            return '<%s>%s</%s>\n' % (self._tag, contents, self._tag)
    def validate(self, deep=1):
        if not self.min_length <= len(self) <= self.max_length:
            raise LengthError, self.length_message % self._tag
        if deep:
            for item in self: item.validate()
    def checkitem(self, item):
        if not match_type(item,[self._type]):
            raise ValidityError, \
                  "Items in %s must be of type %s (not %s)" % \
                  (self._tag, self._type, type(item))
    def lift(self, items):
        # To be friendly, a list of items can be lifted into a list
        # or the type itself.  Extra nice is that we also "lift" a
        # single item into a list of items
        if type(items) not in (tuple, list):
            items = [items]
        lifted = []
        for item in items:
            if match_type(item,[self._type]):
                lifted.append(item)
            elif Seq in self._type.__bases__:
                lifted.append(LiftSeq(self._type, items))
            else:
                lifted.append(self._type(item))
        return lifted

class Some(Quantification):
    length_message = "List <%s> must have length >= 1"
    min_length = 1
    max_length = maxint

class Maybe(Quantification):
    length_message = "List <%s> must have length zero or one"
    min_length = 0
    max_length = 1

class Any(Quantification):
    length_message = "List <%s> may have any length"
    min_length = 0
    max_length = maxint

