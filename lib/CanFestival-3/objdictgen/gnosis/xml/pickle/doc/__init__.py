from os import sep
d = sep.join(__file__.split(sep)[:-1])+sep
_= lambda f: open(d+f).read().rstrip()
l= lambda f: _(f).split('\n')

HOWTO     = _('HOWTO')
COPYRIGHT = _('COPYRIGHT')
HISTORY   = _('HISTORY')
SECURITY  = _('SECURITY')
TODO      = _('TODO')
VERSION   = _('VERSION')

AUTHOR    = l('AUTHOR')
THANKS_TO = l('THANKS_TO')
__doc__ = HOWTO

