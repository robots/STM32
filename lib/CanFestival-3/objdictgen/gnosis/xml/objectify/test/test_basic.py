"Read and print and objectified XML file"

import sys
from gnosis.xml.objectify import XML_Objectify, pyobj_printer, EXPAT, DOM

if len(sys.argv) > 1:
    for filename in sys.argv[1:]:
        for parser in (DOM, EXPAT):
            try:
                xml_obj = XML_Objectify(filename, parser=parser)
                py_obj = xml_obj.make_instance()
                print pyobj_printer(py_obj).encode('UTF-8')
                print "++ SUCCESS ( using", parser, ")"
                print "="*50
            except:
                raise
                print "++ FAILED ( using", parser, ")"
                print "="*50
else:
    print "Please specify one or more XML files to Objectify."


