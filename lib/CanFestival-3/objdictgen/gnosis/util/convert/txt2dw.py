#!/usr/bin/python
__oneliner__="Convert ASCII source files for XML presentation"
__longdoc__="""

  This program is not yet particularly smart, and will produce
  undefined output (or even traceback) if the source file does
  not meet expected format.  With time, it may get better about
  this.

    Usage: python txt2dw.py < MyArticle.txt > MyArticle.xml

------------------------------------------------------------------------
Expected input format for [SMART_ASCII]

      #--- Paragraph rules: ---#
      - Title occurs on first line of document, unindented and in
        all caps.
      - Subtitle occurs on second line, unindented and in mixed
        case.
      - Name, affiliation, date occur, unindented and in mixed
        case, on lines 4-6.
      - Section headings are preceded by two blank lines,
        unindented, in all caps, followed by one line of 72
        dashes and one blank line.
      - Regular text paragraphs are block style, and are indented
        two spaces.
      - Block quotations are indented four spaces, rather than
        the two of original text.
      - Code samples are indented six spaces (with internal
        indentation of code lines in the proper relative
        position).
      - Code samples may begin with a line indicating a title for
        that block.  If present, this title is indented the same
        six spaces as the rest of the block, and begins and ends
        with a pound sign ('#').  Dashes are used to fill space
        within the title for ASCII asthetics.
-
      #--- Character rules: ---#
      - All character markup has the pattern:
            whitespace-symbol-words(s)-symbol-whitespace
        Examples are given, and this can be searched for
        programmatically.  The use of character markup applies
        *only* to text paragraphs, *not* to code samples!
      - Asterisks are used for an inflectional emphasis.  For
        example, "All good boys *deserve* fudge."  This would
        typically be indicated typographically with boldface or
        italics.
      - Underscores are used for book/journal citation.  For
        example, "Knuth's _Art of Computer Programming_ is
        essential."  This would typically be indicated
        typographically with italics or underline.
      - Single-stroke is used to indicate filenames and function
        names.  For example, "Every C program has a 'main()'
        function."  This might be indicated typographically by a
        fixed font, by boldface, or simply by single-quotes.
      - Braces are used to indicate a module, package or library.
        For example, "The [cre] module will replace [re] in
        Python 1.6."  This will probably be indicated
        typographically as a fixed font.
      - Double-stroke is used as either inline quotation or scare
        quotes.  For example, "It may not be as "easy" as
        suggested."  In either case, typographic quotes are
        probably the best format; italics would make some sense
        also.
      - Parenthesis are used, and should be preserved as is.
      - Angle brackets and curly brackets have no special meaning
        yet.  I may choose to use those if there is something I
        think the above forms do not capture.
      - Em-dashes, diacritics, ligatures, and typographic
        quotations are not available, and standard ASCII
        approximations are used.
-
      #--- Miscellany: ---#
      - URL's are automatically transformed into a hotlink.
        Basically, anything that starts with 'http://', 'ftp://',
        'file://' or 'gopher://' looks like a URL to the program.

"""
__doc__=__oneliner__+__longdoc__
__comments__="""
  This script utilizes the services of the Marc-Andre Lemburg's Python
  Highlighter for HTML (v0.5+) [py2html].  [py2html] in turn relies on
  Just van Rossum's [PyFontify] (v.0.3.1+) If these are not present,
  Txt2HTML hopes to degrade gracefully, but will not provide syntax
  highlighting for Python source code.
"""
__author__=["David Mertz (mertz@gnosis.cx)",]
__copyright__="""
    This file is released to the public domain.  I (dqm) would
    appreciate it if you choose to keep derived works under terms
    that promote freedom, but obviously am giving up any rights
    to compel such.
"""
__version__="version 0.1 (August 2001)"

#-- import stuff, or at least try
import sys, re, string, time
from urllib import urlopen
from cStringIO import *
try:
    import dw_colorize
    py_formatter = 1
except:
    py_formatter = 0

#-- Define some XML boilerplate
code_block = """
<heading refname="code1" type="code" toc="yes">%s</heading>
<code type="section">%s</code>"""

#-- End of boilerplate

def main():
    #-- Setup the regex list
    blankln = re.compile("^$")
    headln  = re.compile("\S")        # no indent
    textln  = re.compile("  ? ?\S")   # 1-3 spaces indent
    quoteln = re.compile("    \S")    # 4 spaces indent
    codeln  = re.compile("^      ")   # 6+ spaces indent
    re_list = (blankln, headln, textln, quoteln, codeln)

    #-- Process as needed for input type
    blocks = Make_Blocks(sys.stdin, re_list)
    Process_Blocks(blocks)

def Make_Blocks(fhin, re_list):
    #-- Initialize the globals
    global state, blocks, bl_num, newblock
    state = "HEADER"
    blocks = [""]
    bl_num = 0
    newblock = 1
    #-- Unpack the regex list
    blankln, headln, textln, quoteln, codeln = re_list
    #-- Break the file into relevant chunks
    for line in fhin.readlines():
        line = string.rstrip(line)+'\n'  # Need to normalize line endings!

        if state == "HEADER":         # blank line means new block of ??
            if blankln.match(line):   newblock = 1
            elif textln.match(line):  startText(line)
            elif quoteln.match(line): startQuote(line)
            elif codeln.match(line):  startCode(line)
            else:
                if newblock: startHead(line)
                else: blocks[bl_num] += line
        elif state == "TEXT":         # blank line means new block of ??
            if blankln.match(line):   newblock = 1
            elif headln.match(line):  startHead(line)
            elif quoteln.match(line): startQuote(line)
            elif codeln.match(line):  startCode(line)
            else:
                if newblock: startText(line)
                else: blocks[bl_num] += line
        elif state == "QUOTE":        # blank line means new block of ??
            if blankln.match(line):   newblock = 1
            elif headln.match(line):  startHead(line)
            elif textln.match(line):  startText(line)
            # do not transition quote->code without a blank line
            # elif codeln.match(line):  startCode(line)
            else:
                if newblock:  startQuote(line)
                else: blocks[bl_num] += line
        elif state == "CODE":         # blank line does not change state
            if blankln.match(line):   blocks[bl_num] = blocks[bl_num] + line
            elif headln.match(line):  startHead(line)
            elif textln.match(line):  startText(line)
            else: blocks[bl_num] += line
        else:
            raise ValueError, "unexpected input block state: "+state
    return blocks

def Process_Blocks(blocks):
    # Process all blocks, then write out headers and body
    print '<?xml version="1.0" encoding="UTF-8"?>'
    print '<?xml-stylesheet'
    print '      href="http://gnosis.cx/publish/programming/dW.css"'
    print '      type="text/css"?>'
    print '<article ratings="auto" toc="auto">'

    # Title elements
    head = Detag(blocks[1].replace('[HEAD]',''))
    maintitle, subtitle = head.split('\n')[:2]
    series, paper = maintitle.split(':')
    #--
    print '  <seriestitle>%s</seriestitle>' % series
    print '  <papertitle>%s</papertitle>'   % Typography(paper)
    print '  <subtitle>%s</subtitle>'       % Typography(subtitle)

    # Author and category elements
    author = Detag(blocks[2].replace('[HEAD]',''))
    name, status, date = author.split('\n')[:3]
    jobtitle, company = status.split(',',1)
    month, year = date.split()
    bio = blocks.pop().replace('[TEXT]','').split('\n')
    # Expect: {Picture of Author:  http://gnosis.cx/cgi-bin/img_dqm.cgi}
    biopic = bio[0].split(':',1)[1].strip()[:-1]
    biotxt = Detag('  '+'\n  '.join(bio[1:-1]))
    #--
    print '  <author company="%s"'  % company
    print '          jobtitle="%s"' % jobtitle
    print '          name="%s">'    % name
    print '    <img src="%s" />'    % biopic
    print '%s'                      % URLify(Typography(Detag(biotxt)))
    print '  </author>'
    print '  <date month="%s" year="%s" />' % (month,year)
    print '  <zone name="xml" />'
    print '  <meta name="KEYWORDS" content="Mertz" />'

    # The abstract goes here
    block = blocks[3]
    fixabstract(block[6:])

    # Massage each block as needed
    for block in blocks[4:]:
        if   block[:6]=='[CODE]': fixcode(block[6:])
        elif block[:6]=='[QUOT]': fixquote(block[6:])
        elif block[:6]=='[TEXT]': fixtext(block[6:])
        elif block[:6]=='[HEAD]': fixhead(block[6:])
        else: raise ValueError, "unexpected block marker: "+block[:6]

    print '</article>'

#-- Functions for start of block-type state
def startHead(line):
    global state, blocks, bl_num, newblock
    state = "HEADER"
    bl_num = bl_num+1
    blocks.append('[HEAD]'+line)
    newblock = 0

def startText(line):
    global state, blocks, bl_num, newblock
    state = "TEXT"
    bl_num = bl_num+1
    blocks.append('[TEXT]'+line)
    newblock = 0

def startQuote(line):
    global state, blocks, bl_num, newblock
    state = "QUOTE"
    bl_num = bl_num+1
    blocks.append('[QUOT]'+line)
    newblock = 0

def startCode(line):
    global state, blocks, bl_num, newblock
    state = "CODE"
    bl_num = bl_num+1
    blocks.append('[CODE]'+line)
    newblock = 0

def fixcode(block, doctype='UNKNOWN'):
    # Some XML preparation
    block = LeftMargin(block)

    # Pull out title if available
    re_title = re.compile('^#\-+ (.+) \-+#$', re.M)
    if_title = re_title.match(block)
    if if_title:
        title = if_title.group(1)
        block = re_title.sub('', block)  # take title out of code
    else: title = ''
    # Process the code block with dw_colorize (if possible and appropriate)
    if py_formatter and (string.count(title,'.py') or
                         string.count(title,'Python') or
                         string.count(title,'python') or
                         string.count(title,'py_') or
                         doctype == 'PYTHON'):
        print ('<p><heading refname="code1" type="code" toc="yes">%s</heading>'
               % Typography(title))
        print '<code type="section">',
        dw_colorize.Parser(block.rstrip()).toXML()
        print '</code></p>'
    # elif the-will-and-the-way-is-there-to-format-language-X:
    # elif the-will-and-the-way-is-there-to-format-language-Y:
    else:
        block = Detag(block)
        print code_block % (Typography(title), block.strip())

def fixquote(block):
    print '<blockquote>\n%s</blockquote>' % URLify(Typography(Detag(block)))

def fixabstract(block):
    print '<abstract>\n%s</abstract>' % URLify(Typography(Detag(block)))

def fixtext(block):
    print '<p>\n%s</p>' % URLify(Typography(NoRule(Detag(block))))

def fixhead(block):
    print '\n%s' % Typography(AdjustCaps(NoRule(Detag(block)))+' ')

#-- Utility functions for text transformation
def AdjustCaps(txt):
    # Bare header is block in ALLCAPS (excluding [module] names)
    non_lit = re.sub("['[].*?[]']", '', txt)
    if non_lit == string.upper(non_lit):
        return ('<heading refname="h1" type="major" toc="yes">%s</heading>\n'
                % capwords(txt))
    else:
        return ('<heading refname="h2" type="minor" toc="yes">%s</heading>\n'
                % URLify(txt))
def capwords(txt):
    """string.capwords does'nt do what we want for 'quoted' stuff"""
    words = string.split(txt)
    for n in range(len(words)):
        if not words[n][0] in "'[": words[n] = string.capwords(words[n])
    return string.join(words, ' ')

def LeftMargin(txt):
    """Remove as many leading spaces as possible from whole block"""
    for l in range(12,-1,-1):
        re_lead = '(?sm)'+' '*l+'\S'
        if re.match(re_lead, txt): break
    txt = re.sub('(?sm)^'+' '*l, '', txt)
    return txt

def Detag(txt):
    txt = string.replace(txt, '&', '&amp;')
    txt = string.replace(txt, '<', '&lt;')
    txt = string.replace(txt, '>', '&gt;')
    return txt

def URLify(txt):
    txt0 = txt
    # Convert special IMG URL's, e.g. {Alt Text: http://site.org/img.png}
    # (don't actually try quite as hard to validate URL though)
    txt = re.sub('(?sm){(.*?):\s*(http://.*)}', '<img alt="\\1" src="\\2" />', txt)

    # Convert regular URL's
    txt = re.sub('(?:[^="])((?:http|ftp|gopher|file)://(?:[^ \n\r<\)]+))(\s)',
                 ' <a href="\\1">\\1</a>\\2', txt)
    return txt

def NoRule(txt):
    return re.compile('^-+$', re.M).sub('', txt)

def Typography(txt):
    # [module] names
    r = re.compile(r"""([\(\s'/">]|^)\[(.*?)\]([<\s\.\),:;'"?!/-])""", re.M | re.S)
    txt = r.sub('\\1<code type="inline"><b>\\2</b></code>\\3',txt)
    # *strongly emphasize* words
    r = re.compile(r"""([\(\s'/"]|^)\*(.*?)\*([\s\.\),:;'"?!/-])""", re.M | re.S)
    txt = r.sub('\\1<b>\\2</b>\\3', txt)
    # -emphasize- words
    r = re.compile(r"""([\(\s'/"]|^)-(.*?)-([\s\.\),:;'"?!/])""", re.M | re.S)
    txt = r.sub('\\1<i>\\2</i>\\3', txt)
    # _Book Title_ citations
    r = re.compile(r"""([\(\s'/"]|^)_(.*?)_([\s\.\),:;'"?!/-])""", re.M | re.S)
    txt = r.sub('\\1<attribution>\\2</attribution>\\3', txt)
    # 'Function()' names
    r = re.compile(r"""([\(\s/"]|^)'(.*?)'([\s\.\),:;"?!/-])""", re.M | re.S)
    txt = r.sub('\\1<code type="inline">\\2</code>\\3', txt)
    return txt

#-- The module level code
if __name__ == '__main__':
    main()
