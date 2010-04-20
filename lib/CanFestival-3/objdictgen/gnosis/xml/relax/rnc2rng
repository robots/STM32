#!/usr/bin/env python
import sys
from rnctree import make_nodetree, token_list
if len(sys.argv) > 1:
    tokens = token_list(open(sys.argv[1]).read())
else:
    tokens = token_list(sys.stdin.read())
root = make_nodetree(tokens)
if len(sys.argv) > 2:
    open(sys.argv[2],'w').write(root.toxml())
else:
    print root.toxml()

