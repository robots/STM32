__oneliner__="Convert ASCII source files for HTML presentation"
__longdoc__="""

  This program is not yet particularly smart, and will produce
  undefined output (or even traceback) if the source file does
  not meet expected format.  With time, it may get better about
  this.

      #------------------- Shell Usage -----------------------#
      Usage: python dmTxt2Html.py [options] [filename]    (or)
             txt2html.cgi [options] [filename]

          -h, /h, -?, /?, ?:   Show this help screen
          -type:<type>:        Set conversion type
                               (see discussion of types)
          -REBUILD_DOCS:       Generate 'txt2html.txt'
          -out:<out>:          Output filename (default STDOUT)
          -proxy:<mode>:       Use proxy element(s) in output
          <STDIN>, -, /:       Input from STDIN (default)

      Proxy modes are:  NAVIGATOR, TRAP_LINKS, ALL, NONE.
      Elements are "navigation bar" at top of pages and
      virtualization of in-page links.  Shell default in NONE.

      Note: CGI is detected by the absence of arguments;, if
            all defaults are wanted, specify STDIN explicitly:

          python txt2html.py - < MyArticle.txt > MyArticle.html
-
      #-------------------- CGI Usage ------------------------#
      Usage: A URL string may be composed manually, but the
             normal usage will be to call txt2html.cgi from an
             HTML form with the fields:  'source', 'preface,
             'type', 'proxy'.  'preface' allows explicit
             overriding of HTTP headers in the returned page,
             normally as a hidden field.  Use with caution (or
             don't use at all, the default is sensible).

      Example: <form method="get" action="http://gnosis.cx/cgi-bin/txt2html.cgi">
               URL: <input type="text" name="source" size=40>
               <input type="submit" name="go" value="Display!"></form>

------------------------------------------------------------------------
Expected input format for [HTML]

  Source HTML is presented unmodified except for the inclusion
  of the Txt2HTML proxy at the top of each page.

Expected input format for [PYTHON]

   Source Python code is marked up with syntax highlighting, but
   no other HTML elements are introduced (no headers, no bold, no
   URLs, etc)

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
__version__="version 0.2 (August 2000)"

#-- import stuff, or at least try
import py2html, sys, re, string, time
from urllib import urlopen
from cStringIO import *
try:
    import py2html
    py_formatter = 1
except:
    py_formatter = 0

#-- Define some HTML boilerplate
cgi_home = "http://gnosis.cx/cgi-bin/"
html_open =\
"""<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML//EN">
<html>
<head>
<title>%s</title>
<style>
             {font-family: helvetica, helv, arial, sans-serif; font-size: 10pt; text-align: left}
em           {font-family: helvetica, helv, arial, sans-serif; font-size: 10pt}
ul, ol, li   {font-family: helvetica, helv, arial, sans-serif; font-size: 10pt; text-align: left}
tt           {font-family: mono, courier}
pre          {font-family: mono; font-size: 9pt}
th           {font-family: mono, courier; font-size: 8pt}

.code-sample {background-color: #CCCCCC}
.sidebar     {background-color: #FFFFCC}
.title       {font-size: medium; color: #3333CC}
.subtitle    {font-size: 9pt; color: #3333CC}
.subhead     {font-size: 12pt}
.input       {background: #FFFFFF; color: #000000; font-weight: normal}
</style>
</head>
<body bgcolor="#ffffff" text="#000000">
"""
html_title = "Automatically Generated HTML"
html_close = "</body></html>"
code_block = \
"""<p><strong>%s</strong>
<table border="0" cellpadding="0" class="code-sample" width="100%%"><tr><td>
%s
</td></tr></table></p>
"""
promo = """
<table cellpadding=0 cellspacing=0 border=0 bgcolor="#000000" width="99%%" align=center>
  <tr><td>
    <table width="100%%" cellspacing=1 cellpadding=3 border=0>
      <form method="get" action='"""+cgi_home+"""txt2html.cgi'>
      <tr><td bgcolor="#cccccc">
        <a href='"""+cgi_home+"""txt2html.cgi?source=../rc/nicelinks.txt'>
        <img src='http://gnosis.cx/rc/eye.gif' border=0 alt="More"
             align="right" align="top"></a>&nbsp;
        URL to convert:
              <input type="text" name="source" size=65 value="%(source)s"><br>
        Conversion Type: <select name="type">
              <option selected>INFER
              <option>SIMPLE
              <option>SMART_ASCII
              <option>FAQ
              <option>PYTHON
              <option>RAW
              <option>HTML </select>&nbsp;
        Proxy-mode: <select name="proxy">
              <option>NAVIGATOR
              <option>TRAP_LINKS
              <option selected>ALL
              <option>NONE </select>&nbsp;
        <input type="submit" name="go" value="Display!">
      </td></tr>
      </form>
      <tr><td bgcolor="#cceecc">
        Generated from <tt>%(source)s</tt> by
        <a href="http://gnosis.cx/download/txt2html.zip"><tt>Txt2Html</tt></a>
      </td></tr>
    </table>
  </td></tr>
</table>
"""
#-- End of boilerplate

def main(cfg_dict):
    """Process input source, as per configuration dictionary.

    Just one value is passed to txt2html.main(): a dictionary of operation
    options.  The dictionary must hold the following keys:

        'source':  URL of source (or local filename) or '<STDIN>'
        'target':  Filename of output or '<STDOUT>'
        'preface': (optional) HTTP header for output, esp. if CGI
        'type':    (of input) 'HTML','SMART_ASCII','PYTHON','SIMPLE',...
        'proxy':   'NAVIGATOR', 'TRAP_LINKS', 'ALL', 'NONE'

    """
    replace = string.replace

    #-- Read the configuration, set options
    source = cfg_dict['source']
    if source == '<STDIN>':
        fhin = sys.stdin
        cfg_dict['source'] = '&lt;STDIN&gt;'
    else:
        try:
            fhin = urlopen(source)
        except:
            ErrReport(source+' could not be opened!', cfg_dict)
            return

    target = cfg_dict['target']
    if target == '<STDOUT>':
        fhout = sys.stdout
    else:
        fhout = open(target, 'w')

    #-- Get intype and the regex list
    intype = cfg_dict['type']
    re_list = Regex_by_filetype(intype)

    #-- Process as needed for input type
    if intype in ['HTML']:
        if cfg_dict.has_key('preface'): fhout.write(cfg_dict['preface'])
        doc = ''
        for line in fhin.readlines():   # Need to normalize line endings!
            doc = doc+string.rstrip(line)+'\n'
        if cfg_dict['proxy'] in ['ALL', 'NAVIGATOR']:   # proxy nav bar
            doc = re.sub('(?im)(<BODY(?:.*?)>)','\\1'+(promo % cfg_dict),doc)
        if cfg_dict['proxy'] <> 'NONE':                 # absolute links
            doc = Absolutize(doc, source)
        if cfg_dict['proxy'] in ['ALL', 'TRAP_LINKS']:  # proxy links
            doc = Proxify(doc, cgi_home+'txt2html.cgi?source=')
        fhout.write(doc)
    elif intype in ['SMART_ASCII']:
        #sys.stderr.write('           start: ' + time.ctime(time.time())+"\n")
        blocks = Make_Blocks(fhin, re_list)
        #sys.stderr.write('     blocks made: ' + time.ctime(time.time())+"\n")
        Process_Blocks(fhout, blocks, cfg_dict, title_block=1)
        #sys.stderr.write('blocks processed: ' + time.ctime(time.time())+"\n")
    elif intype in ['PYTHON', 'FAQ', 'SIMPLE', 'RAW']:
        blocks = Make_Blocks(fhin, re_list)
        Process_Blocks(fhout, blocks, cfg_dict)
    else:
        ErrReport('Something is amiss with filetype detection', cfg_dict)
        return

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
                else: blocks[bl_num] = blocks[bl_num] + line

        elif state == "TEXT":         # blank line means new block of ??
            if blankln.match(line):   newblock = 1
            elif headln.match(line):  startHead(line)
            elif quoteln.match(line): startQuote(line)
            elif codeln.match(line):  startCode(line)
            else:
                if newblock: startText(line)
                else: blocks[bl_num] = blocks[bl_num] + line

        elif state == "QUOTE":        # blank line means new block of ??
            if blankln.match(line):   newblock = 1
            elif headln.match(line):  startHead(line)
            elif textln.match(line):  startText(line)
            # do not transition quote->code without a blank line
            # elif codeln.match(line):  startCode(line)
            else:
                if newblock:  startQuote(line)
                else: blocks[bl_num] = blocks[bl_num] + line

        elif state == "CODE":         # blank line does not change state
            if blankln.match(line):   blocks[bl_num] = blocks[bl_num] + line
            elif headln.match(line):  startHead(line)
            elif textln.match(line):  startText(line)
            else: blocks[bl_num] = blocks[bl_num] + line

        else:
            raise ValueError, "unexpected input block state: "+state

    return blocks

def Process_Blocks(fhout, blocks, cfg_dict, title_block=0):
    # Process all blocks, then write out headers and body
    # (performance might be snappier in CGI if we wrote block-by-block...)

    #-- check for server errors
    re_err = re.compile('^.*<TITLE>.*(404)|(403).*</TITLE>', re.I | re.S | re.M)
    block = string.join(blocks[0:3])
    if re_err.match(block):
        html_title = 'Server reported error on URL'
        re_body = re.compile('<BODY(?:.*?)>(.*)</BODY>', re.I | re.M | re.S)
        body = re_body.match(block).group(1)
    else:
        # Worry about having the right number of blocks
        if len(blocks) == 1:
            blocks = ['',cfg_dict['type'],cfg_dict['source'],
                      'Weird failure parsing source experienced!']
        elif len(blocks) <= 3 or not title_block:
            blocks = ['','Type of file: '+cfg_dict['type'],
                      cfg_dict['source']] + blocks[1:]

        # Give it a nice page title
        html_title = string.join(string.split(blocks[1], '\n'),' -- ')
        html_title = string.replace(html_title, '[HEAD]', '')

        # Massage each block as needed
        body = Titleify(blocks[1]) + Authorify(blocks[2])
        for block in blocks[3:]:
            if   block[:6]=='[CODE]': block = fixcode(block[6:],cfg_dict['type'])
            elif block[:6]=='[QUOT]': block = fixquote(block[6:])
            elif block[:6]=='[TEXT]': block = fixtext(block[6:])
            elif block[:6]=='[HEAD]': block = fixhead(block[6:])
            else: raise ValueError, "unexpected block marker: "+block[:6]
            if cfg_dict['proxy'] in ['ALL', 'TRAP_LINKS']:
                block = Proxify(block, cgi_home+'txt2html.cgi?source=')
            body = body+block

    #-- Output with surrounding document HTML/HTTP
    if cfg_dict.has_key('preface'):
        fhout.write(cfg_dict['preface'])
    fhout.write(html_open % html_title)
    if cfg_dict['proxy'] in ['ALL', 'NAVIGATOR']:
        fhout.write(promo % cfg_dict)
    fhout.write(body)
    fhout.write(html_close)

def ErrReport(mess, cfg_dict):
    if cfg_dict.has_key('preface'):  print cfg_dict['preface']
    print (html_open % html_title)
    if cfg_dict['proxy'] in ['ALL', 'NAVIGATOR']:
        print (promo % cfg_dict)
    print '<h1>'+mess+'</h1>'
    print html_close


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


#-- Functions to massage blocks by type
def Titleify(block):
    title, subtitle, other = ('No Title', '', '')
    block = string.replace(block, '[HEAD]', '')
    #block = string.strip(block)
    block = Typographify(block)
    lines = string.split(block, '\n')
    if len(lines) > 0:
        title = lines[0]
    title = string.capwords(title)
    if len(lines) > 1:
        subtitle = lines[1]
    for i in range(2,len(lines)):
        other = other+'<br>'+lines[i]+'\n'
    block = """
<p><strong class="title">%s</strong><br>
<strong class="subtitle">%s</strong></p>
 %s""" % (title, subtitle, other)
    return block

def Authorify(block):
    block = string.replace(block, '[HEAD]', '')
    #block = string.strip(block)
    lines = string.split(block, '\n')
    block = "<p>" +string.join(map(lambda ln:  ln+'<br>\n', lines))+"</p>\n"
    block = Typographify(block)
    return block

def fixcode(block, doctype):
    # Some HTML preparation
    block = Detag(block)
    block = LeftMargin(block)

    # Pull out title if available
    re_title = re.compile('^#\-+ (.+) \-+#$', re.M)
    if_title = re_title.match(block)
    if if_title:
        title = if_title.group(1)
        block = re_title.sub('', block)  # take title out of code
    else: title = ''
    #block = string.strip(block)      # no surrounding whitespace

    # Process the code block with Py2HTML (if possible and appropriate)
    if py_formatter and (string.count(title,'.py') or
                         string.count(title,'Python') or
                         string.count(title,'python') or
                         string.count(title,'py_') or
                         doctype == 'PYTHON'):
        fh = open('tmp', 'w')
        fh.write(block)
        fh.close()
        py2html.main([None, '-format:rawhtml', 'tmp'])
        block = open('tmp.html').read()
        block = code_block % (title, block)
    # elif the-will-and-the-way-is-there-to-format-language-X:
    # elif the-will-and-the-way-is-there-to-format-language-Y:
    else:
        block = code_block % (title, '<pre>'+block+'</pre>')
    return block

def fixquote(block):
    block = Detag(block)
    block = Typographify(block)
    block = '<blockquote>%s</blockquote>' % block
    block = URLify(block)
    return block

def fixtext(block):
    block = Detag(block)
    block = Rulify(block)
    block = Typographify(block)
    block = '<p>%s</p>\n' % block
    block = URLify(block)
    return block

def fixhead(block):
    block = Detag(block)
    block = NoRule(block)
    #block = string.strip(block)
    block = AdjustCaps(block)
    block = Typographify(block+" ")
    return block


#-- Utility functions for text transformation
def AdjustCaps(txt):
    # Bare header is block in ALLCAPS (excluding [module] names)
    non_lit = re.sub("['[].*[]']", '', txt)
    if non_lit == string.upper(non_lit):
        txt = capwords(txt)
        txt = '<p><strong class="subhead">%s</strong></p>\n' % txt
    else:
        txt = '<h3>%s</h3>' % txt
        txt = URLify(txt)
    return txt

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
    txt = re.sub('(?sm){(.*?):\s*(http://.*)}', '<img src="\\2" alt="\\1">', txt)

    # Convert regular URL's
    txt = re.sub('(?:[^="])((?:http|ftp|gopher|file)://(?:[^ \n\r<\)]+))(\s)',
                 '<a href="\\1">\\1</a>\\2', txt)
    return txt

def Proxify(txt, cgi_prog):
    txt = re.sub("""(?i)(<a href=["']?)""", '\\1'+cgi_prog, txt)
    return txt

def Absolutize(txt, base_href):
    """Convert all relative links to absolute links"""

    # Does base look right (not like <STDIN> or filename, basically)?
    if string.count(base_href, "://"):
        txt = string.replace(txt, '<HEAD>', '<HEAD><base href="'+base_href+'">')
        txt = string.replace(txt, '<head>', '<head><base href="'+base_href+'">')
        base_path = re.sub('(.*/).*', '\\1', base_href)    # extract path portion
        txt = re.sub("""(?im)(<a href=["']?)(?!\w+://)(.*?>)""",
                     '\\1'+base_path+'\\2', txt)
    return txt

def Rulify(txt):
    return re.compile('^-+$', re.M).sub('<hr>', txt)

def NoRule(txt):
    return re.compile('^-+$', re.M).sub('', txt)

try:
    from mxTypographify import Typographify
    sys.stderr.write("** Using mxTypographify **\n")
except ImportError:
    def Typographify(txt):
        # [module] names
        r = re.compile(r"""([\(\s'/">]|^)\[(.*?)\]([<\s\.\),:;'"?!/-])""", re.M | re.S)
        txt = r.sub('\\1<em><code>\\2</code></em>\\3',txt)
        # *strongly emphasize* words
        r = re.compile(r"""([\(\s'/"]|^)\*(.*?)\*([\s\.\),:;'"?!/-])""", re.M | re.S)
        txt = r.sub('\\1<strong>\\2</strong>\\3', txt)
        # -emphasize- words
        r = re.compile(r"""([\(\s'/"]|^)-(.*?)-([\s\.\),:;'"?!/])""", re.M | re.S)
        txt = r.sub('\\1<em>\\2</em>\\3', txt)
        # _Book Title_ citations
        r = re.compile(r"""([\(\s'/"]|^)_(.*?)_([\s\.\),:;'"?!/-])""", re.M | re.S)
        txt = r.sub('\\1<cite>\\2</cite>\\3', txt)
        # 'Function()' names
        r = re.compile(r"""([\(\s/"]|^)'(.*?)'([\s\.\),:;"?!/-])""", re.M | re.S)
        txt = r.sub("\\1<code>\\2</code>\\3", txt)
        return txt

def infer_type(fname):
    lower = string.lower
    if lower(fname[-5:])=='.html' or lower(fname[-4:])=='.htm':
        intype = 'HTML'
    elif fname[-1:] in ['.', '/']:
        intype = 'HTML'
    elif lower(fname[-3:])=='.py' or lower(fname[-4:])=='.cgi':
        intype = 'PYTHON'
    elif lower(fname[-4:])=='.faq':
        intype = 'FAQ'
    elif lower(fname[-4:])=='.txt':
        intype = 'SMART_ASCII'
    else:
        intype = 'RAW'
    return intype

def Regex_by_filetype(intype):
    blankln = re.compile("^$")
    headln  = re.compile("\S")        # no indent
    textln  = re.compile("  ? ?\S")   # 1-3 spaces indent
    quoteln = re.compile("    \S")    # 4 spaces indent
    codeln  = re.compile("^      ")   # 6+ spaces indent
    return (blankln, headln, textln, quoteln, codeln)

#-- Sort out meaning of passed arguments/variables
def ParseArgs(list):
    upper = string.upper
    cfg_dict = {'source': '<STDIN>',
                'target': '<STDOUT>',
                'type': 'INFER',
                'proxy': 'NONE' }

    for item in list:
        if item in ['-h','/h','-?','/?', '?']:  # help screen
            print __doc__; return None
        if item[0] in '/-':                     # a switch!
            if upper(item[1:5]) == 'TYPE':      # set type
                cfg_dict['type'] = upper(item[6:])
            if upper(item[1:6]) == 'PROXY':     # set proxy mode
                cfg_dict['proxy'] = upper(item[7:])
            if upper(item[1:]) == 'REBUILD_DOCS':
                fhtxt = open('txt2html.txt', 'w')
                auth = string.join(__author__, '\n')
                docs = ('DMTXT2HTML.PY\n'+__oneliner__+'\n'+
                        '\n'+auth+'\n'+__version__+'\n'+
                        __copyright__+'\n'+
                        __longdoc__+__comments__)
                fhtxt.write(docs)
                fhtxt.close()
                cfg_dict['source'] = 'txt2html.txt'

            # First addition by Justin Swartz <inode@unix.za.net>, 
            # to address the issue of txt2html idling when either:
            #
            #     * an unrecognized switch was specified,
            #     * the 'item' lacked any trailing characters after
            #       a switch character was specified.    
            else:
                ErrReport('Invalid switch', cfg_dict)
                return
            # End Of Addition. [ Sat Feb 12 23:47:04 SAST 2005 ]
        else:                                   # not switch, set source
            # Second addition by Justin Swartz <inode@unix.za.net>,
            # to address the issue of txt2html processing files in
            # directories
            from os import altsep
            from os import extsep
            from os import pardir
            from os import sep
            if ((pardir in item) | ((extsep + sep) in item) |
                ((extsep + altsep) in item)):
                ErrReport('Directory traversal is not welcome', cfg_dict)
                return
            # End Of Addition. [ Sat Feb 12 23:56:25 SAST 2005 ]
            cfg_dict['source'] = item

    if cfg_dict['type'] == 'INFER':
        cfg_dict['type'] = infer_type(cfg_dict['source'])

    global promo        # fix up proxy navigation bar
    promo = string.replace(promo, cgi_home, '')
    return cfg_dict

def ParseCGI():
    import cgi
    upper = string.upper
    cfg_dict = {'target': '<STDOUT>'}
    sys.stderr = sys.stdout
    form = cgi.FieldStorage()
    if form.has_key('source'):
        cfg_dict['source'] = form['source'].value
    else:
        cfg_dict['source'] = '../rc/txt2html.txt'
    if form.has_key('type') and upper(form['type'].value)<>'INFER':
        cfg_dict['type'] = upper(form['type'].value)
    else:
        cfg_dict['type'] = infer_type(cfg_dict['source'])
    if form.has_key('preface'):             # use with caution!
        cfg_dict['preface'] = form['preface'].value
    else:
        cfg_dict['preface'] = 'Content-type: text/html\n\n'
    if form.has_key('proxy'):
        cfg_dict['proxy'] = form['proxy'].value
    else:
        cfg_dict['proxy'] = 'ALL'
    return cfg_dict

#-- The module level code
if __name__ == '__main__':
    # Set options based on runmode (shell vs. CGI)
    if len(sys.argv) >= 2:
        cfg_dict = ParseArgs(sys.argv[1:])
    else:
        cfg_dict = ParseCGI()

    if cfg_dict:  main(cfg_dict)

