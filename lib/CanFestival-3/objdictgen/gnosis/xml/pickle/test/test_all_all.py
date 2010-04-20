
# run test_all for all installed versions of Python
# on the current machine.
#
# NOTE! You have to run this from the development (not installed)
# tree, since it needs disthelper.

import os, sys

# hardcode location of disthelper
dpath = os.path.abspath('../../../..')
sys.path.insert(0,dpath)

from disthelper.find_python import get_python_verlist

for exe,info in get_python_verlist():
    os.system('%s test_all.py' % exe)
    
