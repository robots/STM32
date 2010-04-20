import sys
from urllib import urlopen, urlencode
if len(sys.argv) == 2:
    cgi = 'http://gnosis.cx/cgi/txt2html.cgi'
    opts = urlencode({'proxy': 'NONE',
                      'source': sys.argv[1]})
    print urlopen(cgi, opts).read()
else:
    print "Please specify URL for Txt2Html conversion"

