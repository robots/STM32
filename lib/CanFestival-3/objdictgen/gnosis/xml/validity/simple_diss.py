from gnosis.xml.validity import *
class figure(EMPTY):      pass
class _mixedpara(Or):     _disjoins = (PCDATA, figure)
class paragraph(Some):    _type = _mixedpara
class title(PCDATA):      pass
class _paras(Some):       _type = paragraph
class chapter(Seq):       _order = (title, _paras)
class dissertation(Some): _type = chapter

