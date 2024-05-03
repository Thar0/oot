# config.py
# SPDX-FileCopyrightText: © 2024 ZeldaRET
# SPDX-License-Identifier: CC0-1.0
#
#
#

import os
from enum import Enum, auto
from dataclasses import dataclass

from disassemble_sequence import MMLVersion, SqSection

# ======================================================================================================================
#   Globals
# ======================================================================================================================

SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
SAMPLECONV_PATH = f"{SCRIPT_DIR}/../sampleconv/sampleconv"

# ======================================================================================================================
#   Version Info
# ======================================================================================================================

class GameVersion(Enum):
    OOT_MQDBG = auto(),
    OOT_N0 = auto(),
    MM_J0 = auto(),
    MM_U0 = auto(),
    MM_E1DBG = auto(),

GAMEVERSION_ALL_OOT = (GameVersion.OOT_MQDBG, GameVersion.OOT_N0)
GAMEVERSION_ALL_MM  = (GameVersion.MM_J0, GameVersion.MM_U0, GameVersion.MM_E1DBG)

@dataclass
class GameVersionInfo:
    version_id        : GameVersion # Game Version Identifier
    mml_version       : MMLVersion # Music Macro Language Version

    audiobank_rom     : int # Audiobank vrom start
    audioseq_rom      : int # Audioseq vrom start
    audiotable_rom    : int # Audiotable vrom start
    soundfont_table   : int # Soundfont table vrom start
    seq_font_table    : int # Sequence font table vrom start
    seq_table         : int # Sequence table vrom start
    sample_bank_table : int # Sample bank table vrom start

VERSION_TABLE = {
    "oot_mqdbg" : GameVersionInfo(GameVersion.OOT_MQDBG, MMLVersion.OOT,
                                  0x19030, 0x44DF0, 0x94870, 0xBCC270, 0xBCC4E0, 0xBCC6A0, 0xBCCD90),
    "oot_n0"    : GameVersionInfo(GameVersion.OOT_N0,    MMLVersion.OOT,
                                  0x0D390, 0x29DE0, 0x79470, 0xB896A0, 0xB89910, 0xB89AD0, 0xB8A1C0),

    "mm_j0"     : GameVersionInfo(GameVersion.MM_J0,     MMLVersion.MM,
                                  0x222F0, 0x48160, 0x995D0, 0xC97130, 0xC973C0, 0xC975D0, 0xC97DE0),
    "mm_u0"     : GameVersionInfo(GameVersion.MM_U0,     MMLVersion.MM,
                                  0x20700, 0x46AF0, 0x97F70, 0xC776C0, 0xC77960, 0xC77B70, 0xC78380),
    "mm_e1dbg"  : GameVersionInfo(GameVersion.MM_E1DBG,  MMLVersion.MM,
                                  0x2B2D0, 0x516C0, 0xA2B40, 0xE0F7E0, 0xE0FA80, 0xE0FC90, 0xE104A0),
}

# ======================================================================================================================
#   Hacks
# ======================================================================================================================

# Some sequences are "handwritten", we don't extract these by default as we want these checked in for documentation.
HANDWRITTEN_SEQUENCES_OOT = (0, 1, 2, 109)
HANDWRITTEN_SEQUENCES_MM  = (0, 1)

# Some bugged soundfonts report the wrong samplebank. Map them to the correct samplebank for proper sample discovery.
FAKE_BANKS = {
    GameVersion.OOT_MQDBG : { 37 : 2 },
    GameVersion.OOT_N0    : { 37 : 2 },
    GameVersion.MM_J0     : { 39 : 2 },
    GameVersion.MM_U0     : { 40 : 2 },
    GameVersion.MM_E1DBG  : { 40 : 2 },
}


# Some audiotable banks have a buffer clearing bug. Indicate which banks suffer from this.
AUDIOTABLE_BUFFER_BUGS = {
    GameVersion.OOT_MQDBG : (0,),
    GameVersion.OOT_N0    : (0,),
}


# Tables have no clear start and end in a sequence. Mark the locations of all tables that appear in sequences.

SEQ_DISAS_HACKS_OOT = {
    # sequence number : ((start offset, number of entries, addend, section_type), ...)
    0 : (
            (0x00E1, 128, 0, SqSection.CHAN),
            (0x0EE3,  80, 0, SqSection.CHAN),
            (0x16D5, 248, 0, SqSection.CHAN),
            (0x315E, 499, 0, SqSection.CHAN),
            (0x5729,  72, 0, SqSection.CHAN),
            (0x5EE5,   8, 0, SqSection.CHAN),
            (0x5FF2, 128, 0, SqSection.CHAN),
        ),
    1 : (
            (0x0192, 20, 0, SqSection.LAYER),
            (0x01BA, 20, 0, SqSection.LAYER),
            (0x01E2, 20, 0, SqSection.LAYER),
            (0x020A, 20, 0, SqSection.LAYER),
            (0x0232, 20, 1, SqSection.LAYER),
            (0x025A, 20, 1, SqSection.LAYER),
            (0x0282, 20, 1, SqSection.LAYER),
        ),
    2 : (
            (0x00BC, 2, 0, SqSection.SEQ),
            (0x00C0, 2, 0, SqSection.ARRAY),
        ),
    109 : (
            (0x0646, 16, 0, SqSection.CHAN),
        ),
}


SEQ_DISAS_HACKS_MM = {
    # sequence number : ((start offset, number of entries, addend, section_type), ...)
    0 : (
            (0x011E,   8, 0, SqSection.TABLE),
            (0x012E, 464, 0, SqSection.CHAN),
            (0x18B2,  48, 0, SqSection.LAYER),
            (0x1990, 112, 0, SqSection.CHAN),
            (0x23D8,   8, 0, SqSection.TABLE),
            (0x23E8, 464, 0, SqSection.CHAN),
            (0x566E,   8, 0, SqSection.TABLE),
            (0x567E, 733, 0, SqSection.CHAN),
            (0xA4C1,  96, 0, SqSection.CHAN),
            (0xB163,  16, 0, SqSection.CHAN),
            (0xB2FE,   8, 0, SqSection.TABLE),
            (0xB30E, 390, 0, SqSection.CHAN),

        ),
    1 : (
            (0x018A, 20, 0, SqSection.LAYER),
            (0x01B2, 20, 0, SqSection.LAYER),
            (0x01DA, 20, 0, SqSection.LAYER),
            (0x0202, 20, 0, SqSection.LAYER),
            (0x022A, 20, 1, SqSection.LAYER),
            (0x0252, 20, 1, SqSection.LAYER),
            (0x027A, 20, 1, SqSection.LAYER),
        ),
}


SEQ_DISAS_HACKS = {
    GameVersion.OOT_MQDBG : SEQ_DISAS_HACKS_OOT,
    GameVersion.OOT_N0    : SEQ_DISAS_HACKS_OOT,

    GameVersion.MM_J0     : SEQ_DISAS_HACKS_MM,
    GameVersion.MM_U0     : SEQ_DISAS_HACKS_MM,
    GameVersion.MM_E1DBG  : SEQ_DISAS_HACKS_MM,
}
