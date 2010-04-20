#!/home/gnosis/bin/python

from SimpleXMLRPCServer import SimpleXMLRPCServer
from encode_address import address_like, read_encoding,\
                           encrypt, ascii_huffman_encode,\
                           decrypt, ascii_huffman_decode
KEYFILE = '/home/gnosis/.anonym_passphrase'
HUFFMAN = '/home/gnosis/data/huffman'

encmap, decmap = read_encoding(HUFFMAN)

class Anonym:
    def anonym(self, duration, email):
        pw = open(KEYFILE).read()
        keys = {'perm':pw[:16], 'month':pw[16:32],
                'week':pw[32:48], 'day':pw[48:64] }
        try:
            key = keys[duration]
            if not address_like(email):
                raise ValueError
            email = email.upper()
            enc = ascii_huffman_encode(email, encmap, encrypt, key)
            anonym = '.'+enc+'@gnosis.cx'
        except:
            anonym = "Invalid email or duration (try .help())"
        return anonym
    def help(self):
        return ("Email address must have right general pattern;\n"
                "Duration must be in (perm, month, week, day)")

if __name__ == '__main__':
    server = SimpleXMLRPCServer(('', 8000))
    server.register_instance(Anonym())
    server.serve_forever()
