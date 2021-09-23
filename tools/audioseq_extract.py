#!/usr/bin/env python3

import struct, os

baserom = open("baserom.z64", "rb").read()
audioseq = open("baserom/Audioseq", "rb").read()
seqheader = baserom[0xBCC6A0:][:0x6F0]
num, = struct.unpack(">H", seqheader[:2])

os.makedirs("assets/audio/seqs/", exist_ok=True)

print(num)
for i in range(num):
    data = seqheader[(i+1)*16 : (i+2)*16]
    offset, length = struct.unpack(">II", data[:8])
    # print(f"Sequence {i}: OFFSET={offset:08X} LEN={length:08X}")
    data = audioseq[offset : offset + length]
    with open(f"assets/audio/seqs/{i}.m64", "wb") as f:
        f.write(data)

for i in range(num):
    print(f"Sequence {i}")
    # TODO don't os.system this
    os.system(f"python3 tools/seq_decoder.py assets/audio/seqs/{i}.m64 AudioSeq_{i} > assets/audio/seqs/{i}.s")
