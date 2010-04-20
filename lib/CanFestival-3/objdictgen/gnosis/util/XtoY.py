import re, sys
class SecurityError(Exception): pass
from types import *

pat_fl = r'[-+]?(((((\d+)?[.]\d+|\d+[.])|\d+)[eE][+-]?\d+)|((\d+)?[.]\d+|\d+[.]))'
re_float = re.compile(pat_fl+'$')
re_zero = '[+-]?0$'
pat_int = r'[-+]?[1-9]\d*'
re_int  = re.compile(pat_int+'$')
pat_flint = '(%s|%s)' % (pat_fl, pat_int)    # float or int
re_long = re.compile(r'[-+]?\d+[lL]'+'$')
re_hex  = re.compile(r'([-+]?)(0[xX])([0-9a-fA-F]+)'+'$')
re_oct  = re.compile(r'([-+]?)(0)([0-7]+)'+'$')
pat_complex = r'(%s)?[-+]%s[jJ]' % (pat_flint, pat_flint)
re_complex = re.compile(pat_complex+'$')
pat_complex2 = '(%s):(%s)' % (pat_flint, pat_flint)
re_complex2 = re.compile(pat_complex2+'$')

def aton(s):
    #-- massage the string slightly
    s = s.strip()
    while s[0] == '(' and s[-1] == ')': # remove optional parens
        s = s[1:-1]

    #-- test for cases
    if re.match(re_zero, s): return 0

    if re.match(re_float, s): return float(s)

    if re.match(re_long, s): return long(s)

    if re.match(re_int, s): return int(s)

    m = re.match(re_hex, s)
    if m:
        n = long(m.group(3),16)
        if n < sys.maxint: n = int(n)
        if m.group(1)=='-': n = n * (-1)
        return n

    m = re.match(re_oct, s)
    if m:
        n = long(m.group(3),8)
        if n < sys.maxint: n = int(n)
        if m.group(1)=='-': n = n * (-1)
        return n

    if re.match(re_complex, s): return complex(s)

    if re.match(re_complex2, s):
        r, i = s.split(':')
        return complex(float(r), float(i))

    raise SecurityError, \
          "Malicious string '%s' passed to to_number()'d" % s

# we use ntoa() instead of repr() to ensure we have a known output format
def ntoa(n):
    "Convert a number to a string without calling repr()"
    if isinstance(n,IntType):
        s = "%d" % n
    elif isinstance(n,LongType):
        s = "%ldL" % n
    elif isinstance(n,FloatType):
        s = "%.17g" % n
        # ensure a '.', adding if needed (unless in scientific notation)
        if '.' not in s and 'e' not in s:
            s = s + '.'
    elif isinstance(n,ComplexType):
        # these are always used as doubles, so it doesn't
        # matter if the '.' shows up
        s = "%.17g:%.17g" % (n.real,n.imag)
    else:
        raise ValueError, \
              "Unknown numeric type: %s" % repr(n)
    return s

def to_number(s):
    "Convert a string to a number without calling eval()"
    try:
        return aton(s)
    except SecurityError:
        return None
