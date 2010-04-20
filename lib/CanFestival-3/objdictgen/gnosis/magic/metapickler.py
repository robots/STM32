"""Add Serialization to classes

MetaPickler Example:

    Python 2.2 (#0, Dec 24 2001, 18:42:48) [EMX GCC 2.8.1] on os2emx
    Type "help", "copyright", "credits" or "license" for more information.
    >>> import gnosis.magic
    >>> __metaclass__ = gnosis.magic.MetaPickler
    >>> class Boring:
    ...     def __init__(self):
    ...         self.this = 'that'
    ...         self.spam = 'eggs'
    ...     def print_spam(self):
    ...         print self.spam
    ...
    >>> boring = Boring()
    >>> boring.print_spam()
    eggs
    >>> print boring.dumps()
    <?xml version="1.0"?>
    <!DOCTYPE PyObject SYSTEM "PyObjects.dtd">
    <PyObject module="__main__" class="Boring" id="1276364">
    <attr name="this" type="string" value="that" />
    <attr name="spam" type="string" value="eggs" />
    </PyObject>
"""
class MetaPickler(type):
    "Metaclass for gnosis.xml.pickle serialization"
    def __init__(cls, name, bases, dict):
        from gnosis.xml.pickle import dumps
        super(MetaPickler, cls).__init__(name, bases, dict)
        setattr(cls, 'dumps', dumps)
MetaXMLPickler = MetaPickler

class MetaYamlDump(type):
    "Metaclass for yaml serialization"
    def __init__(cls, name, bases, dict):
        import yaml
        super(MetaYamlDump, cls).__init__(name, bases, dict)
        setattr(cls, 'dumps', lambda self: yaml.dump(self))
        setattr(cls, '__str__', lambda self: "")

class MetaPyPickler(type):
    "Metaclass for cPickle serialization"
    def __init__(cls, name, bases, dict):
        from cPickle import dumps
        super(MetaPyPickler, cls).__init__(name, bases, dict)
        setattr(cls, 'dumps', lambda self: dumps(self))

class MetaPrettyPrint(type):
    "Metaclass for pretty printing"
    def __init__(cls, name, bases, dict):
        from pprint import pformat
        super(MetaPrettyPrint, cls).__init__(name, bases, dict)
        def dumps(self):
            return `self`+'\n'+pformat(self.__dict__)
        setattr(cls, 'dumps', dumps)

