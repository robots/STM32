
# Unicode->XML legality testing & helper functions.
#
# frankm@hiwaay.net

# For detailed information on the motivation for
# this module, see "All About Python and Unicode" at:
#
#    http://boodebr.org/python/pyunicode/index.html

#
# Notes:
#	 Python 2.0.x and 2.1.x have a fatal bug in that they
#	 are unable to read their own byte-compiled versions of
#	 certain unicode literals -- specifically those in the \ud800-\udfff
#	 range. I presume this is due to them being illegal as single
#	 codes. Anyways, Python 2.2 and up have fixed this bug, but
#	 I have used workarounds in the code here for compatibility.
#
#	 So, in several places you'll see I've used unichr() instead of
#	 coding the u'' directly due to this bug. I'm guessing that
#	 might be a little slower.
#

__all__ = ['usplit','is_legal_xml','is_legal_xml_char']

import re

# define True/False if this Python doesn't have them (only
# used in this file)
try:
    a = True
except:
    True = 1
    False = 0

def usplit( uval ):
    """
    Split Unicode string into a sequence of characters.
    \U sequences are considered to be a single character.

    You should assume you will get a sequence, and not assume
    anything about the type of sequence (i.e. list vs. tuple vs. string).
    """

    # The commented code (below) *ALMOST* works -- If Python was compiled to store
    # \U as a single char, you can almost return 'uval' directly, without
    # manual splitting. Unfortunately, you can handcode a 2-char sequence,
    # ie.:
    #		u'\ud800\udc00'
    #
    # .. and it will still be stored as 2-chars. Therefore, the full
    # split logic below is required.
    #
    #	# -*- BROKEN, but almost works -*-
    #	if len(u'\U0010ffff') == 1:
    #	  # this Python was compiled to store \U as single chars,
    #	  # so 'for c in uval' works directly; there is no need to split it.
    #	  return uval
    
    clist = []
    i = 0
    while i < len(uval):
        # if the first character is in range (0xd800 - 0xdbff) and
        # the second character is in range (0xdc00 - 0xdfff), then
        # it is a 2-character encoding
        if len(uval[i:]) > 1 and \
               uval[i] >= unichr(0xD800) and uval[i] <= unichr(0xDBFF) and \
               uval[i+1] >= unichr(0xDC00) and uval[i+1] <= unichr(0xDFFF):
               
            # it's a two character encoding
            clist.append( uval[i:i+2] )
            i += 2
        else:
            # it's a one character encoding
            clist.append( uval[i] )
            i += 1

    return clist

# I put this in a function so the text would be a docstring
def make_illegal_xml_regex():
    """	
    I want to define a regexp to match *illegal* characters.
    That way, I can do "re.search()" to find a single character,
    instead of "re.match()" to match the entire string. [Based on
    my assumption that .search() would be faster in this case.]

    Here is a verbose map of the XML character space (as defined
    in section 2.2 of the XML specification):
    
         u0000 - u0008		   = Illegal
         u0009 - u000A		   = Legal
         u000B - u000C		   = Illegal
         u000D				   = Legal
         u000E - u0019		   = Illegal
         u0020 - uD7FF		   = Legal
         uD800 - uDFFF		   = Illegal (See note!)
         uE000 - uFFFD		   = Legal
         uFFFE - uFFFF		   = Illegal
         U00010000 - U0010FFFF = Legal (See note!)
    
    Note:
    
       The range U00010000 - U0010FFFF is coded as 2-character sequences
       using the codes (D800-DBFF),(DC00-DFFF), which are both illegal
       when used as single chars, from above.
    
       Python won't let you define \U character ranges, so you can't
       just say '\U00010000-\U0010FFFF'. However, you can take advantage
       of the fact that (D800-DBFF) and (DC00-DFFF) are illegal, unless
       part of a 2-character sequence, to match for the \U characters.
    """

    # First, add a group for all the basic illegal areas above
    re_xml_illegal = u'([\u0000-\u0008\u000b-\u000c\u000e-\u0019\ufffe-\uffff])'

    re_xml_illegal += u"|"

    # Next, we know that (uD800-uDBFF) must ALWAYS be followed by (uDC00-uDFFF),
    # and (uDC00-uDFFF) must ALWAYS be preceded by (uD800-uDBFF), so this
    # is how we check for the U00010000-U0010FFFF range. There are also special
    # case checks for start & end of string cases.

    # I've defined this oddly due to the bug mentioned at the top of this file
    re_xml_illegal += u'([%s-%s][^%s-%s])|([^%s-%s][%s-%s])|([%s-%s]$)|(^[%s-%s])' % \
                      (unichr(0xd800),unichr(0xdbff),unichr(0xdc00),unichr(0xdfff),
                       unichr(0xd800),unichr(0xdbff),unichr(0xdc00),unichr(0xdfff),
                       unichr(0xd800),unichr(0xdbff),unichr(0xdc00),unichr(0xdfff))

    return re.compile( re_xml_illegal )

c_re_xml_illegal = make_illegal_xml_regex()

def is_legal_xml( uval ):
    """
    Given a Unicode object, figure out if it is legal
    to place it in an XML file.
    """
    return (c_re_xml_illegal.search( uval ) == None)

def is_legal_xml_char( uchar ):
    """
    Check if a single unicode char is XML-legal.
    (This is faster that running the full 'is_legal_xml()' regexp
    when you need to go character-at-a-time. For string-at-a-time
    of course you want to use is_legal_xml().)

    USAGE NOTE:
       If you want to use this in a 'for' loop,
       make sure use usplit(), e.g.:
          
       for c in usplit( uval ):
          if is_legal_xml_char(c):
                 ... 

       Otherwise, the first char of a legal 2-character
       sequence will be incorrectly tagged as illegal, on
       Pythons where \U is stored as 2-chars.
    """

    # due to inconsistencies in how \U is handled (based on
    # how Python was compiled) it is shorter to test for
    # illegal chars than legal ones, and invert the result.
    #
    # (as one example: (u'\ud900' > u'\U00100000') can be True,
    # depending on how Python was compiled. Testing for illegal chars
    # lets us stick with the single char sequences (all 2-char
    # sequences are legal for XML).

    if len(uchar) == 1:
        return not \
               (
               (uchar >= u'\u0000' and uchar <= u'\u0008') or \
               (uchar >= u'\u000b' and uchar <= u'\u000c') or \
               (uchar >= u'\u000e' and uchar <= u'\u0019') or \
               # always illegal as single chars
               (uchar >= unichr(0xd800) and uchar <= unichr(0xdfff)) or \
               (uchar >= u'\ufffe' and uchar <= u'\uffff')
               )
    elif len(uchar) == 2:
        # all 2-char codings are legal in XML
        # (this looks weird, but remember that even after calling
        # usplit(), \U00010000 is STILL len() of 2, usplit() just
        # made it a single listitem
        return True
    
    else:
        raise Exception("Must pass a single character to is_legal_xml_char")
    
