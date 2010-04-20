#!/home/gnosis/bin/python

from os import popen
from mailbox import PortableUnixMailbox as Mailbox
from email.Parser import Parser

MBOX = '/path/to/incoming/mail' 
KEYFILE = '/path/to/the/keyfile'

blurb = """
====================================================================
==  The below message was forwarded to to you via a Gnosis-Anon   == 
==  pseudonym. See http://gnosis.cx/anon/ for more information    ==
====================================================================

""".lstrip()

mine, anon = [], []
mbox = Mailbox(open(MBOX), Parser().parse)

#-- first, categorize incoming messages
for mess in mbox:
    _to = mess.get('To','ADMIN')
    if _to.startswith('.'):
        anon.append(mess)
    else:
        mine.append(mess)

#-- second, quickly write back the personal messages
mbox = open(MBOX,'w')
for mess in mine:
    mbox.write(mess.as_string(unixfrom=1))
mbox.close()


#-- third, we have time to play with the anonymized recipients
from encode_address import *
encmap, decmap = read_encoding('huffman')
pw = open(KEYFILE).read()
keys = [pw[i:i+16] for i in range(0,64,16)]

for mess in anon:
    _to = mess['To']
    anonym = _to[1:].split('@')[0]
    for key in keys:   # attempt each key in desc disposal duration
        realname = ascii_huffman_decode(anonym, decmap, decrypt, key)
        if address_like(realname):
            subj = mess['Subject']
            del mess['To']
            del mess['Received']
            send = popen('mail -s "%s //s" %s' % (subj,realname) ,'w')
            send.write(blurb)
            send.write(mess.as_string())
            send.close()
            break

print "Content-Type: text/plain"
print
print "Forwarded pseudonymous email"


