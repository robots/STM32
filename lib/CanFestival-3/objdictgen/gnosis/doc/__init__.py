from os import sep
d = sep.join(__file__.split(sep)[:-1])+sep
_ = lambda f: open(d+f).read().rstrip()
l = lambda f: _(f).split('\n')

ANNOUNCE  = _('gnosis_xml_util.announce')
LICENSE   = _('LICENSE')

__doc__   = """Articles and other general documents about Gnosis package"""
