#!/home/gnosis/bin/python

KEYFILE = '/path/to/the/keyfile'
HTML = '../anon/index.html'

import cgi
query = cgi.FieldStorage()
try:
    email = query.getvalue('email','test@test.lan')
    duration = query.getvalue('duration', 'Unknown')
except:
    email = "Invalid"
    duration = "Unknown"

from encode_address import address_like, read_encoding,\
                           encrypt, ascii_huffman_encode,\
                           decrypt, ascii_huffman_decode
encmap, decmap = read_encoding('huffman')
pw = open(KEYFILE).read()
keys = {'perm':pw[:16], 'month':pw[16:32], 'week':pw[32:48], 
        'day':pw[48:64], 'Unknown':'Weak key'}
key = keys[duration]
try:
    if not address_like(email):
        raise ValueError
    enc = ascii_huffman_encode(email.upper(), encmap, encrypt, key)
    anonym = '.'+enc+'@gnosis.cx'
    verify = ascii_huffman_decode(enc, decmap, decrypt, key)
except:
    anonym = "Invalid email address"
    verify = "Cannot verify"

html = open(HTML).read()
for s in (email, anonym, verify, duration):
    html = html.replace('[VALUE]', s, 1)

print "Content-Type: text/html"
print
print html

