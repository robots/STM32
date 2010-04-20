#!/usr/bin/env python

# Define the tokenizer for RELAX NG compact syntax
# This file released to the Public Domain by David Mertz
import lex
tokens = tuple('''
  ELEM ATTR EMPTY TEXT KEYWORD LITERAL ANNOTATION COMMENT
  BEG_PAREN END_PAREN BEG_BODY END_BODY EQUAL NAME CHOICE SEQ
  INTERLEAVE ANY SOME MAYBE WHITESPACE TODO DATATAG PATTERN
  DEFAULT_NS NS DATATYPES NS_ANNOTATION START DEFINE
  '''.split())

reserved = {
   'element'    : 'ELEM',
   'attribute'  : 'ATTR',
   'empty'      : 'EMPTY',
   'text'       : 'TEXT',
   'div'        : 'TODO',
   'external'   : 'TODO',
   'grammar'    : 'TODO',
   'include'    : 'TODO',
   'inherit'    : 'TODO',
   'list'       : 'TODO',
   'mixed'      : 'TODO',
   'notAllowed' : 'TODO',
   'parent'     : 'TODO',
   'string'     : 'TODO',
   'token'      : 'TODO',
}

def t_START(t):
    r"(?im)^start\s*=\s*.*$"
    t.value = t.value.split('=')[1].strip()
    return t

def t_DEFINE(t):
    r"(?im)^[\w-]+\s*="
    t.value = t.value.split('=')[0].strip()
    return t

def t_ANNOTATION(t):
    r"(?im)^\#\# .*$"
    t.value = t.value[3:]
    return t

def t_COMMENT(t):
    r"(?im)^\# .*$"
    t.value = t.value[2:]
    return t

def t_DEFAULT_NS(t):
    r"(?im)default\s+namespace\s*=\s*.*$"
    t.value = t.value.split('=')[1].strip()
    return t

def t_DATATYPES(t):
    r"(?im)datatypes\s+xsd\s*=\s*.*$"
    t.value = t.value.split('=')[1].strip()
    return t

def t_DATATAG(t):
    r"xsd:\w+"
    t.value = t.value.split(':')[1]
    return t

def t_PATTERN(t):
    r'{\s*pattern\s*=\s*".*"\s*}'
    t.value = t.value[:-1].split('=')[1].strip()[1:-1]
    return t

def t_NS(t):
    r"(?im)^namespace\s+.*$"
    t.value = t.value.split(None,1)[1]
    return t

def t_ID(t):
    r"[\w:_-]+"
    t.type = reserved.get(t.value,'NAME')    # Check for reserved words
    return t

def t_LITERAL(t):
    r'".+?"'
    t.value = t.value[1:-1]
    return t

t_BEG_PAREN = r"\("
t_END_PAREN = r"\)"
t_BEG_BODY  = r"{"
t_END_BODY  = r"}"
t_EQUAL     = r"="
t_CHOICE    = r"[|]"
t_SEQ       = r","
t_INTERLEAVE= r"&"
t_ANY       = r"[*]"
t_SOME      = r"[+]"
t_MAYBE     = r"[?]"
t_WHITESPACE= r"\s+"
t_ignore = " \t\n\r"

def t_error(t):
    t.skip(1)

def token_list(rnc):
    lex.lex()
    lex.input(rnc)
    ts = []
    while 1:
        t = lex.token()
        if t is None:
            break
        ts.append(t)
    return ts

if __name__=='__main__':
    import sys
    del t_ignore
    tokens = token_list(sys.stdin.read())
    print '\n'.join(map(repr, tokens))
