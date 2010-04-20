#!/usr/bin/python

import sys, glob, gzip
from trigramlib import generate_trigrams, interesting

MAX_PROBS = 100

# Open dictionary of interesting trigrams
interesting = interesting()

def combine_probs(problist):
    product, inverse = 1.0, 1.0
    for p in problist:
        p = {1.0:.999, 0.0:.001}.get(p,p)
        product *= p
        inverse *= (1-p)
    try:
        return product / (product+inverse)
    except ZeroDivisionError:
        return -1

def is_spam(text):
    trigram_probs = []
    for trigram in generate_trigrams(text):
        if interesting.has_key(trigram):
            trigram_probs.append(interesting[trigram])
            #print trigram, trigrams[trigram]
        if len(trigram_probs) > MAX_PROBS: break
    #print "Candidate trigrams:", len(trigram_probs),
    return combine_probs(trigram_probs)

if __name__=='__main__':
    fcount, flagged, error = 0, 0, 0
    for fname in glob.glob(sys.argv[1]):
        if fname[-3:] == '.gz':
            text = gzip.open(fname).read()
        else:
            text = open(fname).read()

        spam_prob = is_spam(text)
        fcount += 1
        if spam_prob > 0.1: # > 0.9: XXX
            #print fname, spam_prob
            flagged += 1
        elif spam_prob == -1:
            error += 1
        else:   # Non-spam
            #print fname, spam_prob
            pass

    print '='*72
    print '-- Flagged:', flagged,
    print '-- Total:', fcount,
    print '-- Errors:', error,
    print "-- %.2f percent spam" % ((100.0*flagged)/fcount)

