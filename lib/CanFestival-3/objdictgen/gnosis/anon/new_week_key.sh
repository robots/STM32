#!/bin/sh
hexdump KEYFILE
cp KEYFILE src
echo `head -c 32 src``head -c 32 /dev/random` | head -c 64 > KEYFILE
rm src
hexdump KEYFILE

