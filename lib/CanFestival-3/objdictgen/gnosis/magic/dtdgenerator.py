"Add DTD auto-generation to gnosis.xml.validity classes"

from sys import maxint
class DTDGenerator(type):
    declarations = []
    def __init__(cls, name, bases, dict):
        import gnosis.xml.validity, types
        super(DTDGenerator, cls).__init__(name, bases, dict)
        def with_internal_subset(self):
            decl = "<?xml version='1.0'?>"
            dtd = DTDGenerator.dtd()
            xml = self.__str__()
            root = self.__class__.__name__
            return "%s\n<!DOCTYPE %s [\n%s\n]>\n%s" % (decl,root,dtd,xml)
        cls.with_internal_subset = with_internal_subset
        DTDGenerator.add_decl(cls, name)

    def add_decl(self, cls, name):
        import gnosis.xml.validity as gxv
        add = self.declarations.append
        if name in dir(gxv):
            "skip the classes directly in gnosis.xml.validity"
        elif issubclass(cls, gxv.PCDATA):
            add((name,'#PCDATA',None))
        elif issubclass(cls, gxv.EMPTY):
            add((name, 'EMPTY', None))
        elif issubclass(cls, gxv.Or):
            items = []
            for item in map(lambda c: c.__name__, cls._disjoins):
                if item=='PCDATA': items.append('#PCDATA')
                else: items.append(item)
            add((name, 'Or', tuple(items)))
        elif issubclass(cls, gxv.Seq):
            items = map(lambda c: c.__name__, cls._order)
            add((name, 'Seq', tuple(items)))
        elif issubclass(cls, gxv.Quantification):
            # This relies on each Quantification descendent being
            # right depth... something more general would be better
            grandparent = cls.__bases__[0].__bases__[0].__name__
            add((name, grandparent, cls._type.__name__))
    add_decl = classmethod(add_decl)

    def dtd(self):
        decl_list, defer = [], {}
        add = decl_list.append
        # On first pass, generate templates of final declarations
        for (name, type_, data) in self.declarations:
            if name.startswith("_"):
                defer[name] = (type_, data)
            elif type_ == '#PCDATA':
                add("<!ELEMENT %s (#PCDATA)>" % name)
            elif type_ == 'EMPTY':
                add("<!ELEMENT %s EMPTY>" % name)
            elif type_ == 'ANY':
                add("<!ELEMENT %s ANY>" % name)
            elif type_ == 'Or':
                add("<!ELEMENT %s (%s)>" % (name, "|".join(data)))
            elif type_ == 'Some':
                add("<!ELEMENT %s (%s)+>" % (name, data))
            elif type_ == 'Maybe':
                add("<!ELEMENT %s (%s)?>" % (name, data))
            elif type_ == 'Any':
                add("<!ELEMENT %s (%s)*>" % (name, data))
            elif type_ == 'Seq':
                add("<!ELEMENT %s (%s)>" % (name, ",".join(data)))

        # On second pass, generate substitutions
        subs = {}
        for name, (type_, data) in defer.items():
            if   type_=='Seq':  subs[name] = "(%s)" % ",".join(data)
            elif type_=='Or':   subs[name] = "(%s)" % "|".join(data)
            elif type_=='Some': subs[name] = "%s+" % data
            elif type_=='Maybe':subs[name] = "%s?" % data
            elif type_=='Any':  subs[name] = "%s*" % data

        # 3rd pass: may be substitutable names inside other subs
        def expand(key, dct):
            prev = ""
            while prev != dct[key]:
                prev = dct[key]
                for old, new in dct.items():
                    dct[key] = dct[key].replace(old, new)
        map(lambda x: expand(x, subs), subs.keys())

        # On final pass, substitute-in to the declarations
        for decl, i in zip(decl_list, xrange(maxint)):
            for name, sub in subs.items():
                decl = decl.replace(name, sub)
            decl_list[i] = decl
        return '\n'.join(decl_list)
    dtd = classmethod(dtd)

