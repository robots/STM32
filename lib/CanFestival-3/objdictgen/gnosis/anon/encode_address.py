#!/home/gnosis/bin/python

#-- Let's define the en/decryption functions (crude for now)
def xor_key(key):
    "Produce 100 bytes of key-based pseudo-random bits"
    import sha

    s = ""
    for _ in range(5):
        s += sha.sha(s+key).digest()
    return s

def crypt(key, s, decrypt):
    """En/decrypt a string with a key

    The technique here is still weak.  We crypt with rotor to get a
    reasonable diffusion, then xor against an SHA-based key for stronger
    confusion, then rotor again to further diffuse the bytes
    """
    import rotor
    r = rotor.newrotor(key, 12)
    if decrypt: rot = r.decrypt
    else:       rot = r.encrypt
    xor = xor_key(key)
    s = rot(s)
    chars = [chr(ord(s[i])^ord(xor[i])) for i in range(len(s))]
    s = "".join(chars)
    s = rot(s)
    return s
encrypt = lambda k, s, dec=0: crypt(k, s, dec)
decrypt = lambda k, s, dec=1: crypt(k, s, dec)

#-- Huffman encoding and decoding of strings
def read_encoding(fname):
    """Read in a huffman character map from a file

    Each line of the encoding should have the form:
        1213 E  :1111
    That is, an ocurrence count (generated from a corpus), whitespace,
    a character, whitespace, a colon followed by a string of 1s and 0s
    """
    enc = open(fname).readlines()
    encmap = {}
    for line in enc:
        flds = line.split()
        if len(flds)==3 and flds[2][0]==":":
            encmap[flds[1]] = tuple(map(int, flds[2][1:]))
    decmap = dict([(bits,c) for (c,bits) in encmap.items()])
    return encmap, decmap

def ascii_huffman_encode(s, encmap, encrypt=lambda k,s:s, key=None):
    "Compress a string as 7-bit ASCII"
    bits, bytes = [], []
    for c in s:
        bits.extend(encmap[c])
    toX8 = (8-len(bits))%8          # we need a whole number of bytes
    bits.extend([0]*toX8)
    for i in range(0,len(bits),8):  # compute bytes from 8 bit chunks
        b0,b1,b2,b3,b4,b5,b6,b7 = bits[i:i+8]
        v = 128*b0 + 64*b1 + 32*b2 + 16*b3 + 8*b4 + 4*b5 + 2*b6 + b7
        bytes.append(chr(v))
    crypt = encrypt(key, "".join(bytes))
    return crypt.encode('base64').rstrip().replace('=','')

hex2bits = {'0':(0,0,0,0),'1':(0,0,0,1),'2':(0,0,1,0),'3':(0,0,1,1),
            '4':(0,1,0,0),'5':(0,1,0,1),'6':(0,1,1,0),'7':(0,1,1,1),
            '8':(1,0,0,0),'9':(1,0,0,1),'a':(1,0,1,0),'b':(1,0,1,1),
            'c':(1,1,0,0),'d':(1,1,0,1),'e':(1,1,1,0),'f':(1,1,1,1)}

def string2bits(s):
    "Convert a string to a bit list, based on the ord of each byte"
    bits = []
    for hexrep in ['%02x' % ord(c) for c in s]:
       for c in hexrep:
            bits.extend(hex2bits[c])
    return bits

def ascii_huffman_decode(enc, decmap, decrypt=lambda k,s:s, key=None):
    "Restore an huffman encoded 7-bit string"
    s = ("%s%s\n" % (enc,"="*(len(enc)%4))).decode('base64')
    plain = decrypt(key, s)
    bits = string2bits(plain)
    decoded, head = [], 0
    for pos in range(len(bits)+1):
        if decmap.has_key(tuple(bits[head:pos])):
            decoded.append(decmap[tuple(bits[head:pos])])
            head = pos
    return "".join(decoded)

#-- General utility functions
def address_like(s):
    import re
    return re.match(r'[\w_+./!-]+@([\w-]+\.)+[A-Za-z]{2,4}', s)

if __name__=='__main__':
    import sys, os
    key = open('ANONYM_PASSPHRASE').readlines()[0]
    encmap, decmap = read_encoding('huffman')
    plain, coded = 0,0

    for addr in sys.stdin.xreadlines():
        addr = addr.strip().upper()
        enc_addr = ascii_huffman_encode(addr, encmap, encrypt, key)
        if not address_like(addr): print addr
        #print len(addr), len(enc_addr), addr, enc_addr
        print ascii_huffman_decode(enc_addr, decmap, decrypt, key)
        plain += len(addr)
        coded += len(enc_addr)

    sys.stderr.write('Encoded bits-per-char %.2f\n' % (8.*coded/plain))

