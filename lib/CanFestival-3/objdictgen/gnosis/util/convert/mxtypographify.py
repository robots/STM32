#!/usr/bin/python

"""mxTextTools version of Typographify() in dmTxt2Html.py
    
The hope here is that this version will be dramatically faster 
than the regular-expression based version.
"""
from TextTools import *
import string, re

#-- List to contain all words with adjusted markup
ws = []
head_pos = None
loops = 0

#-- Define "emitter" callbacks for each output format
def emit_misc(tl,txt,l,r,s): ws.append(txt[l:r])
def emit_func(tl,txt,l,r,s): ws.append('<code>'+txt[l+1:r-1]+'</code>')
def emit_modl(tl,txt,l,r,s): ws.append('<em><code>'+txt[l+1:r-1]+'</code></em>')
def emit_emph(tl,txt,l,r,s): ws.append('<em>'+txt[l+1:r-1]+'</em>')
def emit_strg(tl,txt,l,r,s): ws.append('<strong>'+txt[l+1:r-1]+'</strong>')
def emit_titl(tl,txt,l,r,s): ws.append('<cite>'+txt[l+1:r-1]+'</cite>')
def jump_count(tl,txt,l,r,s):
    global head_pos, loops
    loops = loops+1
    if head_pos is None: head_pos = r
    elif head_pos == r: 
        raise "InfiniteLoopError", txt[l-20:l]+'{'+txt[l]+'}'+txt[l+1:r+15]
    else: head_pos = r
def emit_debug(tl,txt,l,r,s): 
    ws.append(txt[l:r])
    print '<<', txt[l:r], '>>'
    
#-- What can appear inside, and what can be, markups?
punctuation = "`!@#$%^&*()_-+=|\{}[]:;'<>,.?/"+'"'
punct_set = set(punctuation)
markable = alphanumeric+whitespace+"`!@#$%^&()+=|\{}:;<>,.?/"+'"'
markable_func = set(markable+"*-_[]")
markable_modl = set(markable+"*-_'")
markable_emph = set(markable+"*_'[]")
markable_strg = set(markable+"-_'[]")
markable_titl = set(markable+"*-'[]")

markups = "-*'[]_"
markup_set = set(markups)

# What can precede and follow markup phrases? 
darkins = '(/"'
leadins = whitespace+darkins      # might add from "-*'[]_"  
darkouts = '/.),:;?!"'   
darkout_set = set(darkouts)
leadouts = whitespace+darkouts    # for non-conflicting markup
leadout_set = set(leadouts)

# What can appear inside plain words?
wordish = alphanumeric+'{}/@#$%^&-_+=|\><'+darkouts
word_set = set(wordish)
wordinit = alphanumeric+"$#+\<.&{"+darkins 
wordinit_set = set(wordinit)

#-- Define the word patterns (global so as to do it only at import)

# Special markup
def markup_struct(lmark, rmark, callback, markables, x_post="-"):
    struct = \
      ( callback, Table+CallTag,
        ( (None, Is, lmark),                 # Starts with left marker
          (None, AllInSet, markables),       # All stuff marked (that looks right) 
          (None, Is, rmark),                 # Ends with right maker
          (None, IsInSet, leadout_set,+2,+1),# EITHER: postfixed with lead-out 
      (None, Skip, -1,+1, MatchOk),      # ..give back trailing lead-out char
      (None, IsIn, x_post, MatchFail),   # OR: special case postfix
          (None, Skip, -1,+1, MatchOk)       # ..give back special trailing char
    )
      )
    return struct
funcs   = markup_struct("'", "'", emit_func, markable_func)
modules = markup_struct("[", "]", emit_modl, markable_modl)
emphs   = markup_struct("-", "-", emit_emph, markable_emph, x_post="") 
strongs = markup_struct("*", "*", emit_strg, markable_strg) 
titles  = markup_struct("_", "_", emit_titl, markable_titl)

# All the stuff not specially marked
plain_words = \
 ( ws, Table+AppendMatch,               # AppendMatch is only -slightly- 
   ( (None, IsInSet,                    #   faster than emit_misc callback
        wordinit_set, MatchFail),   # Must start with word-initial
     (None, Is, "'",+1),                # May have apostrophe next
     (None, AllInSet, word_set,+1),     # May have more word-internal
     (None, Is, "'", +2),               # May have trailing apostrophe
     (None, IsIn, "st",+1),             # May have [ts] after apostrophe 
     (None, IsInSet,
        darkout_set,+1, MatchOk),   # Postfixed with dark lead-out
     (None, IsInSet, 
        whitespace_set, MatchFail), # Give back trailing whitespace
     (None, Skip, -1)
   )
 )

# Catch some special cases 
bullet_point = \
 ( ws, Table+AppendMatch,
   ( (None, Word+CallTag, "* "),       # Asterisk bullet is a word
   )
 )  

horiz_rule = \
 ( None, Table,
   ( (None, Word, "-"*50),             # 50 dashes in a row
     (None, AllIn, "-"),               # More dashes
   )
 )

into_mark = \
 ( ws, Table+AppendMatch,             # Special case where dark lead-in
   ( (None, IsInSet, set(darkins)),   # is followed by markup char
     (None, IsInSet, markup_set),
     (None, Skip, -1)                 # Give back the markup char
   )
 )

stray_punct = \
 ( ws, Table+AppendMatch,              # Pickup any cases where multiple
   ( (None, IsInSet, punct_set),       # punctuation character occur 
     (None, AllInSet, punct_set),      # alone (followed by whitespace)
     (None, IsInSet, whitespace_set), 
     (None, Skip, -1)                  # Give back the whitespace
   )
 )

leadout_eater = (ws, AllInSet+AppendMatch, leadout_set)
 
# Tag all the (possibly marked-up) words 
tag_words = \
 ( bullet_point+(+1,),
   horiz_rule + (+1,),
   into_mark  + (+1,),
   stray_punct+ (+1,),
   emphs   + (+1,),
   funcs   + (+1,),
   strongs + (+1,),
   modules + (+1,),
   titles  + (+1,),
   into_mark+(+1,),
   plain_words +(+1,),                 # Since file is mostly plain words, can
   leadout_eater+(+1,-1),              # shortcut by tight looping (with escape)
   (jump_count, Skip+CallTag, 0),      # Check for infinite loop
   (None, EOF, Here, -13)              # Check for EOF
 ) 

def Typographify(txt):
    global ws
    ws = []    # clear the list before we proceed
    tag(txt, tag_words, 0, len(txt), ws)
    return string.join(ws, '')

if __name__ == '__main__':
    import sys, time
    txt = open(sys.argv[1]).read()
    start = time.time()
    mx_txt = Typographify(txt)
    sys.stderr.write('*** TextTools processing ***\n')
    sys.stderr.write(sys.argv[1]+' processed in %.3f seconds' 
                     % (time.time()-start)+'\n')
    sys.stderr.write('%d full tagging loops\n' % loops) 
    print mx_txt

    # Just for comparison, let us time the [re] version being replaced
    def reTypographify(txt):
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
    start = time.time()
    re_txt = reTypographify(txt)
    sys.stderr.write('*** re processing ***\n')
    sys.stderr.write(sys.argv[1]+' processed in %.3f seconds' 
                     % (time.time()-start)+'\n')

    

    


    


