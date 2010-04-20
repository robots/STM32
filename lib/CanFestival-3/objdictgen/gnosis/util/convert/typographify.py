import os
from sys import stdin, stdout, stderr
from simpleparse import generator
from mx.TextTools import TextTools

input = stdin.read()
decl = open('typographify.def').read()
from typo_html import codes
parser = generator.buildParser(decl).parserbyname('para')
taglist = TextTools.tag(input, parser)
for tag, beg, end, parts in taglist[1]:
    if tag == 'plain':
        stdout.write(input[beg:end])
    elif tag == 'markup':
        markup = parts[0]
        mtag, mbeg, mend = markup[:3]
        start, stop = codes.get(mtag, ('<!-- unknown -->','<!-- /unknown -->'))
        stdout.write(start + input[mbeg+1:mend-1] + stop)
stderr.write('parsed %s chars of %s\n' %  (taglist[-1], len(input)))

