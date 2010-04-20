import uu, sys, string, cStringIO, os
fh = open(sys.argv[1], "rb")
insize = os.path.getsize(sys.argv[1])

print "--- Extracting UUEncoded segments from", sys.argv[1], "---"

segment = []
end_last_part = ""
in_segment = 0
readsize = 1024 * 1024
fname = 'INIT'

while 1:
    part = fh.read(readsize)
    if not part: break

    lines = string.split(end_last_part+part, "\n")
    end_last_part = lines[-1]
    for line in lines[:-1]:
        if not line:                            # empty line
            pass
        elif line[:5] == "begin":               # beginning of segment
            in_segment = 1
            segment = [line]
            fname = string.strip(line[9:])
        elif string.strip(line) == "end":       # end of segment
            in_segment = 0
            segment.append(line)
            chunk = string.join(segment, "\n")
            infile = cStringIO.StringIO(chunk+"\n")
            if os.path.isfile(fname):   # avoid decoding existing file
                print "File already exists:", fname
            elif string.find(fname, ".r") > -1:
                print "Skipping presumed RAR archive"
            else:
                try:                    # might not have nice segment
                    uu.decode(infile)
                    print "Extracted file:", fname+"!"
                except:
                    print "Problem extracting segment..."
        else:
            if in_segment:                          # segment content
                if len(segment) > 20000:
                    segment = [segment[0]]
                    print "Segment truncated to avoid memory overrun!"
                segment.append(line)


