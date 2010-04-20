#!/bin/sh
hexdump KEYFILE
cp KEYFILE src
echo `head -c 16 src``head -c 48 /dev/random` | head -c 64 > KEYFILE
rm src
hexdump KEYFILE

