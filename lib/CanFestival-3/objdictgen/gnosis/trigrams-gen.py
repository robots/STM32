#!/usr/bin/python

import sys
from trigramlib import read_trigrams, generate_trigrams, write_trigrams

def report(trigrams, top=10):
    print "Trigrams analyzed:", len(trigrams)
    print "="*72
    trigram_list = []
    for trigram,(spam,good) in trigrams.items():
        ratio = float(spam)/(spam+good)
        if spam+good > 100 and not (0.9 > ratio > 0.1):
            trigram_list.append((ratio,trigram, spam, good))
    trigram_list.sort()
    for best, worst in zip(trigram_list[:top],trigram_list[-top:]):
        print "%.2f %s: (%6d | %6d )    %.2f %s: (%6d | %6d )" % (best+worst)

if __name__=='__main__':
    # Open an existing partial model, if it exists
    trigrams = read_trigrams('trigrams')

    # Process each instruction line from STDIN
    for line in sys.stdin:
        print line.strip(),
        option, fname = line.split()
        if option not in ('SPAM','GOOD'):
            print "Specify input type: SPAM/GOOD"
            sys.exit()
        for trigram in generate_trigrams(open(fname).read()):
            if not trigrams.has_key(trigram):
                trigrams[trigram] = [0,0]
            if option=='SPAM':
                trigrams[trigram][0] += 1
            elif option=='GOOD':
                trigrams[trigram][1] += 1
        print '...', len(trigrams)
    write_trigrams(trigrams,'trigrams')
    report(trigrams)
