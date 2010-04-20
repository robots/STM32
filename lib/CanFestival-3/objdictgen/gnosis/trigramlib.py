"Support functions for trigram analysis"
from __future__ import generators
import string, cPickle

def simplify(text):
    ident = [chr(x) for x in range(256)]
    for n in (10,11,12,13):
        ident[n] = chr(n+128)
    ident = ''.join(ident)
    hibit = ''.join([chr(x) for x in range(128,256)])
    return string.translate(text, ident, hibit)

def simplify_indexer(text):
    from gnosis.indexer import TextSplitter
    ts = TextSplitter()
    ts.initSplitter()
    return " ".join(ts.text_splitter(text, casesensitive=1))

def simplify_null(text):
    ident = ''.join(map(chr, range(256)))
    return text.translate(ident, '\n\r')

def generate_trigrams(text, simplify=simplify):
    "Iterator on trigrams in (simplified) text"
    text = simplify(text)
    for i in xrange(len(text)-3):
        yield text[i:i+3]

def read_trigrams(fname):
    try:
        trigrams = {}
        for line in open(fname):
            trigram = line[:3]
            spam,good = map(lambda s: int(s,16), line[3:].split(':'))
            trigrams[trigram] = [spam,good]
        return trigrams
    except IOError:
        return {}

def write_trigrams(trigrams, fname):
    fh = open(fname,'w')
    for trigram,(spam,good) in trigrams.items():
        print >> fh, '%s%x:%x' %(trigram,spam,good)
    fh.close()

def interesting(rebuild=0):
    "Identify the interesting trigrams"
    if not rebuild:
        try:
            return cPickle.load(open('interesting-trigrams','rb'))
        except IOError:
            pass
    trigrams = read_trigrams('trigrams')
    interesting = {}
    for trigram,(spam,good) in trigrams.items():
        ratio = float(spam)/(spam+good)
        if spam+good >= 10:
            if ratio < 0.05 or ratio > 0.95:
                interesting[trigram] = ratio
    cPickle.dump(interesting, open('interesting-trigrams','wb'), 1)
    return interesting


