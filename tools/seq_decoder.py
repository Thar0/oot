#!/usr/bin/env python3
import sys

def join(*args):
    ret = {}
    for arg in args:
        ret.update(arg)
    return ret

control_flow_commands = {
    0xff: ['end'],
    0xfe: ['delay1'],
    0xfd: ['delay', 'var'],
    0xfc: ['call', 'addr'],
    0xfb: ['jump', 'addr'],
    0xfa: ['beqz', 'addr'],
    0xf9: ['bltz', 'addr'],
    0xf8: ['loop', 'u8'],
    0xf7: ['loopend'],
    0xf6: ['break'],
    0xf5: ['bgez', 'addr'],
    0xf4: ['jumprel', 'reladdr8'],
    0xf3: ['beqzrel', 'reladdr8'],
    0xf2: ['bltzrel', 'reladdr8'],
}

commands = {}
commands['seq'] = join(control_flow_commands, {
    # non-arg commands
    0xf1: ['reservenotes', 'u8'],
    0xf0: ['unreservenotes'],
    0xdf: ['transpose', 's8'],
    0xde: ['transposerel', 's8'],
    0xdd: ['settempo', 'u8'],
    0xdc: ['addtempo', 's8'],
    0xdb: ['setvol', 'u8'],
    0xda: ['changevol', 's8'],
    0xd7: ['initchannels', 'hex16'],
    0xd6: ['disablechannels_noop', 'hex16'],
    0xd5: ['setmutescale', 's8'],
    0xd4: ['mute'],
    0xd3: ['setmutebhv', 'hex8'],
    0xd2: ['setshortnotevelocitytable', 'addr'],
    0xd1: ['setshortnotedurationtable', 'addr'],
    0xd0: ['setnoteallocationpolicy', 'u8'],
    0xce: ['rand', 'u8'],
    0xcd: ['dyncall', 'addr'],
    0xcc: ['setval', 'u8'],
    0xc9: ['bitand', 'u8'],
    0xc8: ['subtract', 'u8'],
    0xc7: ['writeseq', 'u8', 'addr'],
    0xc6: ['stopscript'],
    0xc5: ['setscriptcounter', 'u16'],
    0xc4: ['startseq', 'u8', 'u8'],
    # arg commands
    0x00: ['testchdisabled', 'bits:4'],
    0x40: ['disablechannel', 'bits:4'],
    0x50: ['ioreadvalsub', 'bits:4'],
    0x60: ['asyncload', 'bits:4', 'u8', 'u8'],
    0x70: ['iowriteval', 'bits:4'],
    0x80: ['ioreadval', 'bits:4'],
    0x90: ['startchannel', 'bits:4', 'addr'],
    0xA0: ['startchannelrel', 'bits:4', 'reladdr16'],
    0xB0: ['loadseq', 'bits:4', 'u8', 'addr'],
})

commands['chan'] = join(control_flow_commands, {
    # non-arg commands
    0xf1: ['reservenotes', 'u8'],
    0xf0: ['unreservenotes'],
    0xee: ['pitchbend2', 's8'],
    0xed: ['sethilogain', 'u8'],
    0xeb: ['setbankandinstr'],
    0xea: ['stopscript'],
    0xe9: ['setnotepriority', 'u8'],
    0xe5: ['setreverbindex', 'u8'],
    0xe4: ['dyncall'],
    0xe3: ['setvibratodelay', 'u8'],
    0xe2: ['setvibratoextentlinear', 'u8', 'u8', 'u8'],
    0xe1: ['setvibratoratelinear', 'u8', 'u8', 'u8'],
    0xe0: ['setvolscale', 'u8'],
    0xdf: ['setvol', 'u8'],
    0xde: ['freqscale', 'u16'],
    0xdd: ['setpan', 'u8'],
    0xdc: ['setpanmix', 'u8'],
    0xdb: ['transpose', 's8'],
    0xda: ['setenvelope', 'addr'],
    0xd9: ['setdecayrelease', 'u8'],
    0xd8: ['setvibratoextent', 'u8'],
    0xd7: ['setvibratorate', 'u8'],
    0xd6: ['setupdatesperframe_unimplemented', 'u8'],
    0xd4: ['setreverb', 'u8'],
    0xd3: ['pitchbend', 's8'],
    0xd2: ['setsustain', 'u8'],
    0xd1: ['setnoteallocationpolicy', 'u8'],
    0xd0: ['stereoheadseteffects', 'u8'],
    0xcf: ['writeseqlarge', 'addr'],
    0xce: ['setvallarge', 'hex16'],
    0xcd: ['disablechannel', 'u8'],
    0xcc: ['setval', 'u8'],
    0xcb: ['readseq', 'addr'],
    0xca: ['setmutebhv', 'hex8'],
    0xc9: ['bitand', 'u8'],
    0xc8: ['subtract', 'u8'],
    0xc7: ['writeseq', 'u8', 'addr'],
    0xc6: ['setbank', 'u8'],
    0xc5: ['dynsetdyntable'],
    0xc4: ['largenoteson'],
    0xc3: ['largenotesoff'],
    0xc2: ['setdyntable', 'addr'],
    0xc1: ['setinstr', 'u8'],
    0xbd: ['randrangelarge', 'u16', 'u16'],
    0xbc: ['addlarge', 'u16'],
    0xbb: ['unk_bb', 'u8', 'u16'],
    0xba: ['setdurationrandomvariance', 'u8'],
    0xb9: ['setvelocityrandomvariance', 'u8'],
    0xb8: ['rand', 'u8'],
    0xb7: ['randlarge', 'u16'],
    0xb6: ['readdyntable'],
    0xb5: ['readdyntablelarge'],
    0xb4: ['dynsetdyntablelarge'],
    0xb3: ['loadfilter', 'u8'],
    0xb2: ['dynreadseqlarge', 'addr'],
    0xb1: ['clearfilter'],
    0xb0: ['setfilter', 'addr'],
    # arg commands
    0x00: ['delayshort', 'bits:4'],
    0x10: ['loadsample', 'bits:3', 'addr'],
    0x18: ['loadsamplelarge', 'bits:3', 'addr'],
    0x20: ['startchannel', 'bits:4', 'addr'],
    0x30: ['iowriteval2', 'bits:4', 'u8'],
    0x40: ['ioreadval2', 'bits:4', 'u8'],
    0x50: ['ioreadvalsub', 'bits:4'],
    0x60: ['ioreadval', 'bits:4'],
    0x70: ['iowriteval', 'bits:3'],
    0x78: ['setlayerrel', 'bits:3', 'reladdr16'],
    0x80: ['testlayerfinished', 'bits:3'],
    0x88: ['setlayer', 'bits:3', 'addr'],
    0x90: ['freelayer', 'bits:3'],
    0x98: ['dynsetlayer', 'bits:3'],
})

commands_layer_base = join(control_flow_commands, {
    # non-arg commands
    0xc0: ['delay', 'var'],
    0xc1: ['setshortnotevelocity', 'u8'],
    0xc2: ['transpose', 's8'],
    0xc3: ['setshortnotedefaultplaypercentage', 'var'],
    0xc4: ['continuousnoteson'],
    0xc5: ['continuousnotesoff'],
    0xc6: ['setinstr', 'u8'],
    0xc7: ['portamento', 'hex8', 'u8', 'u8'],
    0xc8: ['disableportamento'],
    0xc9: ['setshortnoteduration', 'u8'],
    0xca: ['setpan', 'u8'],
    0xcb: ['setenvelope', 'addr', 'u8'],
    0xcc: ['ignoredrumpan'],
    0xcd: ['setstereo', 'u8'],
    0xce: ['pitchbend2', 's8'],
    0xcf: ['setreleaserate', 'u8'],
    # arg commands
    0xd0: ['setshortnotevelocityfromtable', 'bits:4'],
    0xe0: ['setshortnotedurationfromtable', 'bits:4'],
})
del commands_layer_base[0xfd]

commands['layer_large'] = join(commands_layer_base, {
    0x00: ['note0', 'bits:6', 'var', 'u8', 'u8'],
    0x40: ['note1', 'bits:6', 'var', 'u8'],
    0x80: ['note2', 'bits:6', 'u8', 'u8'],
})

commands['layer_small'] = join(commands_layer_base, {
    0x00: ['smallnote0', 'bits:6', 'var'],
    0x40: ['smallnote1', 'bits:6'],
    0x80: ['smallnote2', 'bits:6'],
})

branches = ['jump', 'beqz', 'bltz', 'bgez', 'jumprel', 'beqzrel', 'bltzrel']

def valid_cmd_for_nbits(cmd_list, nbits):
    for arg in cmd_list:
        if arg.startswith('bits:'):
            return int(arg.split(':')[1]) == nbits
    return nbits == 0

print_end_padding = False
if "--print-end-padding" in sys.argv:
    print_end_padding = True
    sys.argv.remove("--print-end-padding")

if len(sys.argv) not in [2,3]:
    print(f"Usage: {sys.argv[0]} (--emit-asm-macros | input.m64 seqname)")
    sys.exit(0)

if sys.argv[1] == "--emit-asm-macros":
    def comment(text):
        print(f"# {text}")

    comment("Macros for disassembled sequence files. This file was automatically generated by seq_decoder.py.")
    comment("To regenerate it, run: ./tools/seq_decoder.py --emit-asm-macros > include/seq_macros.inc")
    print()

    def print_hword(x):
        print(f"    .byte {x} >> 8, {x} & 0xff")

    def emit_cmd(key, op, cmd):
        mn = cmd[0]
        args = cmd[1:]
        param_names = []
        param_list = []
        bits_param_name = None
        for i, arg in enumerate(args):
            param_name = chr(97 + i)
            param_names.append(param_name)
            param_list.append(param_name + ("=0" if arg.endswith(":ign") else ""))
            if arg.startswith("bits:"):
                bits_param_name = param_name
        print(f".macro {key}_{mn} {', '.join(param_list)}".rstrip())
        if bits_param_name is not None:
            print(f"    .byte {hex(op)} + \\{bits_param_name}")
        else:
            print(f"    .byte {hex(op)}")
        for arg, param_name in zip(args, param_names):
            if arg.startswith('bits:'):
                pass
            elif arg in ['s8', 'u8', 'hex8']:
                print(f"    .byte \\{param_name}")
            elif arg in ['u16', 'hex16']:
                print_hword("\\" + param_name)
            elif arg == 'addr':
                print_hword(f"(\\{param_name} - sequence_start)")
            elif arg == 'reladdr8':
                print(f"    .byte (\\{param_name} - reladdr\\@)")
                print(f"reladdr\\@:")
            elif arg == 'reladdr16':
                print_hword(f"(\\{param_name} - reladdr\\@)")
                print(f"reladdr\\@:")
            elif arg == 'var_long':
                print(f"    var_long \\{param_name}")
            elif arg == 'var':
                print(f"    var \\{param_name}")
            else:
                raise Exception("Unknown argument type " + arg)
        print(".endm")
        print()

    def emit_env_cmd(op, cmd):
        mn = cmd[0]
        param_names = []
        param_list = []
        for i, arg in enumerate(cmd[1:]):
            param_name = chr(97 + i)
            param_names.append(param_name)
            param_list.append(param_name + ("=0" if arg.endswith(":ign") else ""))
        print(f".macro envelope_{mn} {', '.join(param_list)}".rstrip())
        if op is not None:
            print(f"    .byte {hex(op >> 8)}, {hex(op & 0xff)}")
        for param in param_names:
            print_hword("\\" + param)
        print(".endm\n")

    for key in ['seq', 'chan', 'layer']:
        comment(f"{key} commands\n")
        if key == 'layer':
            cmds = commands['layer_large']
            for op in sorted(commands['layer_small'].keys()):
                if op not in cmds:
                    emit_cmd(key, op, commands['layer_small'][op])
        else:
            cmds = commands[key]
        for op in sorted(cmds.keys()):
            mn = cmds[op][0]
            if mn != 'portamento':
                emit_cmd(key, op, cmds[op])

        if key == 'chan':
            print(".macro chan_setvallargeaddr a")
            print("    .byte 0xce")
            print_hword("(\\a - sequence_start)")
            print(".endm\n")
            print(".macro layer_portamento a, b, c")
            print("    .byte 0xc7, \\a, \\b")
            print("    .if ((\\a & 0x80) == 0)")
            print("        var \\c")
            print("    .else")
            print("        .byte \\c")
            print("    .endif")
            print(".endm\n")
            emit_cmd(key, 0xfd, ['delay_long', 'var_long'])
        if key == 'layer':
            emit_cmd(key, 0xc0, ['delay_long', 'var_long'])
            emit_cmd(key, 0x40, ['note1_long', 'bits:4', 'var_long', 'u8'])

    comment("envelope commands\n")
    emit_env_cmd(0, ['disable', 'u16'])
    emit_env_cmd(2**16-1, ['hang', 'u16:ign'])
    emit_env_cmd(2**16-2, ['goto', 'u16'])
    emit_env_cmd(2**16-3, ['restart', 'u16:ign'])
    emit_env_cmd(None, ['line', 'u16', 'u16'])

    comment("other commands\n")
    print(".macro var_long x")
    print("     .byte (0x80 | (\\x & 0x7f00) >> 8), (\\x & 0xff)")
    print(".endm\n")
    print(".macro var x")
    print("    .if (\\x >= 0x80)")
    print("        var_long \\x")
    print("    .else")
    print("        .byte \\x")
    print("    .endif")
    print(".endm\n")
    print(".macro entry a")
    print_hword("(\\a - sequence_start)")
    print(".endm")
    print(".macro filter a,b,c,d,e,f,g,h")
    print_hword("\\a")
    print_hword("\\b")
    print_hword("\\c")
    print_hword("\\d")
    print_hword("\\e")
    print_hword("\\f")
    print_hword("\\g")
    print_hword("\\h")
    print(".endm")
    sys.exit(0)

filename = sys.argv[1]
seqname = sys.argv[2]

try:
    # should maybe renumber in hex?
    seq_num = int(filename.split('/')[-1].split('_')[0].split('.')[0], 10)
except Exception:
    seq_num = -1

try:
    with open(filename, 'rb') as f:
        data = f.read()
except Exception:
    print("Error: could not open file {filename} for reading.", file=sys.stderr)
    sys.exit(1)

output = [None] * len(data)
alignment = [None] * len(data)
output_instate = [None] * len(data)
label_name = [None] * len(data)
script_start = [False] * len(data)
hit_eof = False
errors = []
seq_writes = []

# Our analysis of large notes mode doesn't persist through multiple channel activations
# For simplicity, we force large notes always instead, which is valid for oot.
force_large_notes = True

def gen_label(ind, tp):
    nice_tp = tp.replace('_small', '').replace('_large', '').replace('lazy', '')
    addr = hex(ind)[2:].upper()
    ret = f".{nice_tp}_{addr}"
    if ind >= len(data):
        errors.append(f"reference to oob label {ret}")
        return ret

    if label_name[ind] is not None:
        return label_name[ind]
    label_name[ind] = ret
    return ret

def gen_mnemonic(tp, b):
    nice_tp = tp.split('_')[0]
    mn = commands[tp][b][0]
    if not mn:
        mn = f"{b:02X}"
    return f"{nice_tp}_{mn}"

decode_list = []

def decode_one(state):
    pos, tp, nesting, large = state
    orig_pos = pos

    if pos >= len(data):
        global hit_eof
        hit_eof = True
        return

    if seq_num == 0 and pos in (0x6197, 0x61BD, 0x6372):
        # unfinished code
        return

    if output[pos] is not None:
        if output_instate[pos] != state:
            errors.append(f"got to {gen_label(orig_pos, tp)} with both state {state} and {output_instate[pos]}")
        return

    def u8():
        nonlocal pos
        global hit_eof
        if pos == len(data):
            hit_eof = True
            return 0
        ret = data[pos]
        pos += 1
        return ret

    def s8():
        ret = u8()
        return ret - 0x100 if ret >= 0x80 else ret

    def u16():
        hi = u8()
        lo = u8()
        return (hi << 8) | lo

    def s16():
        ret = u16()
        return ret - 0x10000 if ret >= 0x8000 else ret

    def peek16():
        nonlocal pos
        ret = u16()
        pos -= 2
        return ret

    def var():
        ret = u8()
        if ret & 0x80:
            ret = (ret << 8) & 0x7f00;
            ret |= u8()
            return (ret, ret < 0x80)
        return (ret, False)

    if tp.endswith('entry'):
        subtp = tp[:-5]
        if subtp == 'layer':
            subtp = 'layer_large'
        addr = u16()
        if subtp != 'lazyseq' and subtp != 'writeseq':
            decode_list.append((addr, subtp, 0, True))
        for p in range(orig_pos, pos):
            output[p] = ''
            output_instate[p] = state
        if subtp == 'writeseq':
            output[orig_pos] = 'entry <fixup>'
            seq_writes.append((orig_pos, addr))
        else:
            output[orig_pos] = 'entry ' + gen_label(addr, subtp)
            if addr < len(data):
                script_start[addr] = True
        return

    if tp == 'envelope':
        a = u16()
        b = u16()
        for p in range(orig_pos, pos):
            output[p] = ''
            output_instate[p] = state
        if a >= 2**16 - 3:
            a -= 2**16
        if a <= 0:
            mn = ['disable', 'hang', 'goto', 'restart'][-a]
            output[orig_pos] = f'envelope_{mn} {b}'
            # assume any goto is backwards and stop decoding
        else:
            output[orig_pos] = f'envelope_line {a} {b}'
            decode_list.append((pos, tp, nesting, large))
        return

    if tp == 'filter':
        filt_str = ", ".join(str(s16()) for _ in range(8))
        for p in range(orig_pos, pos):
            output[p] = ''
            output_instate[p] = state
        output[orig_pos] = f'filter {filt_str}'
        return

    ins_byte = u8()

    cmds = commands[tp]
    nbits = 0
    used_b = ins_byte
    while True:
        if used_b in cmds and valid_cmd_for_nbits(cmds[used_b], nbits):
            break
        used_b &= ~(1 << nbits)
        nbits += 1
        if nbits == 8:
            errors.append(f"unrecognized instruction {hex(ins_byte)} for type {tp} at label {gen_label(orig_pos, tp)}")
            return

    out_mn = gen_mnemonic(tp, used_b)
    out_args = []
    cmd_mn = cmds[used_b][0]
    cmd_args = cmds[used_b][1:]
    long_var = False

    for a in cmd_args:
        if cmd_mn == 'portamento' and len(out_args) == 2 and (int(out_args[0], 0) & 0x80) == 0:
            a = 'var'
        if cmd_mn == 'setvallarge' and seq_num == 0 and peek16() not in (0, 0x7fbc):
            a = 'addr'
            out_mn = "chan_setvallargeaddr"

        if a.startswith('bits:'):
            low_bits = ins_byte & ((1 << nbits) - 1)
            if not (a.endswith(':ign') and low_bits == 0):
                out_args.append(str(low_bits))
        elif a == 'u8':
            out_args.append(str(u8()))
        elif a == 'hex8':
            out_args.append(hex(u8()))
        elif a == 's8':
            v = u8()
            out_args.append(str(v if v < 128 else v - 256))
        elif a == 'u16':
            out_args.append(str(u16()))
        elif a == 'hex16':
            out_args.append(hex(u16()))
        elif a == 'var':
            val, bad = var()
            out_args.append(hex(val))
            if bad:
                long_var = True
        elif a in ('addr', 'reladdr8', 'reladdr16'):
            if a == 'addr':
                v = u16()
            elif a == 'reladdr8':
                v = s8()
                v += pos
            else:
                v = s16()
                v += pos

            kind = 'addr'
            if cmd_mn == 'call':
                kind = tp + '_fn'
            elif cmd_mn in branches:
                kind = tp
            elif cmd_mn in ('startchannel', 'startchannelrel'):
                kind = 'chan'
            elif cmd_mn in ('setlayer', 'setlayerrel'):
                kind = 'layer'
            elif cmd_mn in ('setdyntable', 'dyncall'):
                kind = 'table'
            elif cmd_mn == 'setenvelope':
                kind = 'envelope'
            elif cmd_mn == 'setfilter':
                kind = 'filter'

            if v >= len(data):
                label = gen_label(v, kind)
                out_args.append(label)
                errors.append(f"reference to oob label {label}")
            elif cmd_mn in ('writeseq', 'writeseqlarge'):
                out_args.append('<fixup>')
                seq_writes.append((orig_pos, v))
            else:
                out_args.append(gen_label(v, kind))
                if cmd_mn == 'call':
                    decode_list.append((v, tp, 0, large))
                    script_start[v] = True
                elif cmd_mn in branches:
                    decode_list.append((v, tp, nesting, large))
                elif kind == 'chan':
                    decode_list.append((v, 'chan', 0, force_large_notes))
                    script_start[v] = True
                elif kind == 'layer':
                    if large:
                        decode_list.append((v, 'layer_large', 0, True))
                    else:
                        decode_list.append((v, 'layer_small', 0, True))
                    script_start[v] = True
                elif kind == 'envelope':
                    decode_list.append((v, 'envelope', 0, True))
                    script_start[v] = True
                elif kind == 'filter':
                    decode_list.append((v, 'filter', 0, True))
                    script_start[v] = True
                else:
                    script_start[v] = True
        else:
            raise Exception(f"bad arg kind {a}")

    out_all = out_mn
    if long_var:
        out_all += "_long"
    if out_args:
        out_all += ' '
        out_all += ', '.join(out_args)
    for p in range(orig_pos, pos):
        output[p] = ''
        output_instate[p] = state
    output[orig_pos] = out_all

    if cmd_mn in ['hang', 'jump', 'jumprel']:
        return
    if cmd_mn in ['loop']:
        nesting += 1
    if cmd_mn == 'end':
        nesting -= 1
    if cmd_mn in ['break', 'loopend']:
        nesting -= 1
        if nesting < 0:
            # This is iffy, and actually happens in sequence 0. It will make us
            # return to the caller's caller at function end.
            nesting = 0
    if cmd_mn == 'largenoteson':
        large = True
    if cmd_mn == 'largenotesoff':
        large = False
    if nesting >= 0:
        decode_list.append((pos, tp, nesting, large))

def decode_rec(state, initial):
    if not initial:
        v = state[0]
        gen_label(v, state[1])
        script_start[v] = True
    decode_list.append(state)
    while decode_list:
        decode_one(decode_list.pop())

def main():
    decode_rec((0, 'seq', 0, False), initial=True)

    tables = []
    unused = []

    if seq_num == 0:
        tables = [
            ('chan', 0xE1, 0x80),
            ('chan', 0x1E1, 0x60),
            ('chan', 0xEE3, 0x50),
            ('chan', 0x16D5, 0x80),
            ('chan', 0x17D5, 0x78),
            ('chan', 0x315E, 0x80),
            ('chan', 0x325E, 0x80),
            ('chan', 0x335E, 0x80),
            ('chan', 0x345E, 0x73),
            ('chan', 0x5729, 0x48),
            ('chan', 0x5EE5, 0x8),
            ('chan', 0x5FF2, 0x80),
        ]

        unused = [
            (0x4BE, 'layer_large'),
            (0x6F6, 'layer_large'),
            (0x72C, 'layer_large'),
            (0x839, 'chan'),
            (0x109A, 'layer_large'),
            (0x1C7A, 'envelope'),
            (0x1C86, 'envelope'),
            (0x1C92, 'envelope'),
            (0x205E, 'layer_large'),
            (0x2128, 'layer_large'),
            (0x213D, 'layer_large'),
            (0x3791, 'chan'),
            (0x482F, 'chan'),
            (0x599F, 'chan'),
            (0x59B9, 'layer_large'),
            (0x5B45, 'layer_large'),
            (0x6185, 'chan'),
            (0x61AB, 'chan'),
            (0x6360, 'chan'),
            (0x672C, 'envelope'),
            (0x685C, 'envelope'),
            (0x689C, 'envelope'),
            (0x691C, 'envelope'),
            (0x693C, 'envelope'),
            (0x69EC, 'envelope'),
            (0x6A6C, 'envelope'),
        ]

    elif seq_num == 1:
        tables = [
            ('layer', 0x192, 20),
            ('layer', 0x1BA, 20),
            ('layer', 0x1E2, 20),
            ('layer', 0x20A, 20),
            ('writeseq', 0x232, 20),
            ('writeseq', 0x25A, 20),
            ('writeseq', 0x282, 20),
        ]

    elif seq_num == 2:
        tables = [
            ('lazyseq', 0xC0, 2),
            ('seq', 0xBC, 2),
        ]

    elif seq_num == 109:
        tables = [
            ('chan', 0x646, 16),
        ]
        unused = [
            (0x3F7, 'layer_large'),
            (0x578, 'layer_large'),
            (0x666, 'envelope'),
            (0x66E, 'envelope'),
            (0x67E, 'envelope'),
            (0x6A6, 'envelope'),
            (0x6BA, 'envelope'),
            (0x6E2, 'envelope'),
            (0x70A, 'envelope'),
            (0x736, 'envelope'),
            (0x74E, 'envelope'),
            (0x766, 'envelope'),
            (0x776, 'envelope'),
            (0x782, 'envelope'),
            (0x79A, 'envelope'),
            (0x7A6, 'envelope'),
            (0x7AE, 'envelope'),
            (0x7B6, 'envelope'),
            (0x7C2, 'envelope'),
            (0x7D6, 'envelope'),
            (0x7E6, 'envelope'),
            (0x80E, 'envelope'),
            (0x82A, 'envelope'),
            (0x83A, 'envelope'),
            (0x852, 'envelope'),
            (0x862, 'envelope'),
            (0x896, 'envelope'),
            (0x8AE, 'envelope'),
            (0x8BA, 'envelope'),
        ]

    for (tp, addr, count) in tables:
        for i in range(count):
            decode_rec((addr + 2*i, tp + 'entry', 0, False), initial=True)

    for (addr, tp) in unused:
        gen_label(addr, tp + '_unused')
        decode_rec((addr, tp, 0, force_large_notes), initial=False)

    for (pos, write_to) in seq_writes:
        assert '<fixup>' in output[pos]
        delta = 0
        while output[write_to] == '':
            write_to -= 1
            delta += 1
        tp = output_instate[write_to][1] if output_instate[write_to] is not None else 'addr'
        nice_target = gen_label(write_to, tp)
        if delta:
            nice_target += "+" + str(delta)
        output[pos] = output[pos].replace('<fixup>', nice_target)

    # Add unreachable 'end' markers
    for i in range(1, len(data)):
        if (data[i] == 0xff and output[i] is None and output[i - 1] is not None
                and label_name[i] is None):
            tp = output_instate[i - 1][1]
            if tp in ["seq", "chan", "layer_small", "layer_large"]:
                output[i] = gen_mnemonic(tp, 0xff)

    # Add aligners and strip padding
    for i in range(len(data)):
        if not output[i]:
            continue
        tp = output_instate[i][1] if output_instate[i] else ''
        if tp == 'filter':
            align = 16
        elif tp == 'envelope':
            align = 2
        else:
            continue
        if i % align != 0:
            errors.append(f"{label_name[i]} ({hex(i)}) is unaligned")
            continue
        alignment[i] = align
        for j in range(1, align):
            k = i - j
            if k < 0 or output[k] is not None or data[k] != 0 or label_name[k]:
                break
            output[k] = ""

    # Add 'unused' marker labels
    for i in range(1, len(data)):
        if (output[i] is None and output[i - 1] is not None and label_name[i] is None):
            script_start[i] = True
            gen_label(i, 'unused')

    # Remove up to 15 bytes of padding at the end
    end_padding = 0
    for i in range(len(data)-1, -1, -1):
        if output[i] is not None:
            break
        end_padding += 1
    if end_padding > 15:
        end_padding = 0

    if print_end_padding:
        print(end_padding)
        sys.exit(0)

    print(".include \"seq_macros.inc\"")
    print(".section .rodata")
    print(".balign 0x10")
    print(f".global {seqname}_Start")
    print(f"{seqname}_Start:")
    print(f"sequence_start:")
    print()
    last_tp = ""
    for i in range(len(data) - end_padding):
        if script_start[i] and i > 0:
            print()
        tp = output_instate[i][1] if output_instate[i] else ''
        if tp != last_tp and alignment[i] is not None:
            print(f".balign {alignment[i]}\n")
        last_tp = tp
        if label_name[i] is not None:
            print(f"{label_name[i]}:")
        o = output[i]
        if o is None:
            print(f".byte {hex(data[i])}")
        elif o:
            print(o)
        elif label_name[i] is not None:
            print("<mid-instruction>")
            errors.append(f"mid-instruction label {label_name[i]}")

    print("\n.balign 0x10")
    print(f".global {seqname}_Length")
    print(f".set {seqname}_Length, . - sequence_start")

    if not data:
        print("# empty")
    elif hit_eof:
        errors.append("hit eof!?")

    if errors:
        print(f"[{filename}] errors:", file=sys.stderr)
        for w in errors:
            print(w, file=sys.stderr)

main()
