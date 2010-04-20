from dmTxt2Html import *     # Import the body of 'Txt2Html' code
from urllib import urlopen
import sys

# Check for updated functions (fail gracefully if not fetchable)
try:
    updates = urlopen('http://gnosis.cx/download/t2h_textfuncs.py').read()
    fh = open('t2h_textfuncs.py', 'w')
    fh.write(updates)
    fh.close()
except:
    sys.stderr.write('Cannot currently download Txt2Html updates')

# Import the updated functions (if available)
try:
    from t2h_textfuncs import *
except:
    sys.stderr.write('Cannot import the updated Txt2Html functions')

# Set options based on runmode (shell vs. CGI)
if len(sys.argv) >= 2:
    cfg_dict = ParseArgs(sys.argv[1:])
    main(cfg_dict)
else:
    print "Please specify URL (and options) for Txt2Html conversion"



