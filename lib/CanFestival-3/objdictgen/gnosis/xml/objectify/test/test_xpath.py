from gnosis.xml.objectify import make_instance
from gnosis.xml.objectify.utils import XPath
import sys

xml = '''<foo>
  <bar>this</bar>
  <bar>that</bar>
  <baz a1="fie" a2="fee">
    stuff <bar>fo</bar>
    <bar a1="fiddle">fum</bar>
    and <bam><bar>fizzle</bar></bam>
    more stuff
  </baz>
</foo>
'''

print xml
print
open('xpath.xml','w').write(xml)
o = make_instance(xml)

patterns = '''/bar //bar //* /baz/*/bar
              /bar[2] //bar[2..4]
              //@a1 //bar/@a1 /baz/@* //@*
              baz//bar/text() /baz/text()[3]'''

for pattern in patterns.split():
    print 'XPath:', pattern
    for match in XPath(o, pattern):
        print ' ', match
