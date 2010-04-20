"""Hot-pluggable replacement functions for Txt2Html"""

#-- Functions to massage blocks by type
#def Titleify(block):
#def Authorify(block):
#def fixcode(block, doctype):
#def fixquote(block):
#def fixtext(block):
#def fixhead(block):

#-- Utility functions for text transformation
#def AdjustCaps(txt):
#def capwords(txt):
#def LeftMargin(txt):
#def Detag(txt):
#def URLify(txt):
#def Proxify(txt, cgi_prog):
#def Absolutize(txt, base_href):
#def Rulify(txt):
#def NoRule(txt):
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

#-- Miscellaneous type-specific functions
#def infer_type(fname):
#def Regex_by_filetype(intype):

