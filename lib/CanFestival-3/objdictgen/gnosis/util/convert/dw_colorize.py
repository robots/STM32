"""developerWorks/XML Python Source Colorizer (customized from MoinMoin)"""

# Imports
import cgi, string, sys, cStringIO
import keyword, token, tokenize

_KEYWORD = token.NT_OFFSET + 1
_TEXT    = token.NT_OFFSET + 2

_colors = {
    token.NUMBER:       'black',
    token.OP:           None,
    token.STRING:       'green',
    tokenize.COMMENT:   'red',
    token.NAME:         None,
    token.ERRORTOKEN:   'red',
    _KEYWORD:           'blue',
    _TEXT:              'black',
}

class Parser:
    """ Colorize python source"""
    def __init__(self, raw):
        """ Store the source text"""
        self.raw = string.strip(string.expandtabs(raw))

    def toXML(self):
        """ Parse and send the colored source."""
        # store line offsets in self.lines
        self.lines = [0, 0]
        pos = 0
        while 1:
            pos = string.find(self.raw, '\n', pos) + 1
            if not pos: break
            self.lines.append(pos)
        self.lines.append(len(self.raw))

        # parse the source and write it
        self.pos = 0
        text = cStringIO.StringIO(self.raw)
        try:
            tokenize.tokenize(text.readline, self)
        except tokenize.TokenError, ex:
            msg = ex[0]
            line = ex[1][0]
            print "ERROR: %s %s" % (msg, self.raw[self.lines[line]:])

    def __call__(self, toktype, toktext, (srow,scol), (erow,ecol), line):
        """ Token handler"""
        # calculate new positions
        oldpos = self.pos
        newpos = self.lines[srow] + scol
        self.pos = newpos + len(toktext)

        # handle newlines
        if toktype in [token.NEWLINE, tokenize.NL]:
            sys.stdout.write('\n')
            return

        # send the original whitespace, if needed
        if newpos > oldpos:
            sys.stdout.write(self.raw[oldpos:newpos])

        # skip indenting tokens
        if toktype in [token.INDENT, token.DEDENT]:
            self.pos = newpos
            return

        # map token type to a color group
        if token.LPAR <= toktype and toktype <= token.OP:
            toktype = token.OP
        elif toktype == token.NAME and keyword.iskeyword(toktext):
            toktype = _KEYWORD
        color = _colors.get(toktype, _colors[_TEXT])

        # send text
        toktext = cgi.escape(toktext)
        if not color:
            if toktext: sys.stdout.write(toktext)
        elif color == 'black':
            if toktext: sys.stdout.write('<b>%s</b>' % (toktext))

        else:
            if toktext: sys.stdout.write('<b class="%s">%s</b>'
                                         % (color, toktext))

if __name__ == "__main__":
    # write colorized version of STDIN
    Parser(sys.stdin.read()).toXML()

