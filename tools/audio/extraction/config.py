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
    # OoT
    OOT_NTSC_0_9 = auto(),
    OOT_NTSC_1_0 = auto(),
    OOT_NTSC_1_1 = auto(),
    OOT_NTSC_1_2 = auto(),
    OOT_NTSC_1_2_GW = auto(),
    OOT_PAL_1_0 = auto(),
    OOT_PAL_1_1 = auto(),
    OOT_GC_JP = auto(),
    OOT_GC_JP_CE = auto(),
    OOT_GC_JP_MQ = auto(),
    OOT_GC_US = auto(),
    OOT_GC_US_MQ = auto(),
    OOT_GC_EU = auto(),
    OOT_GC_EU_DBG0 = auto(),
    OOT_GC_EU_DBG1 = auto(),
    OOT_GC_EU_MQ = auto(),
    OOT_GC_EU_MQ_DBG = auto(),
    OOT_IQUE_CN = auto(),
    OOT_IQUE_ZH = auto(),
    # MM
    MM_N64_JP_1_0 = auto(),
    MM_N64_JP_1_1 = auto(),
    MM_N64_US = auto(),
    MM_N64_US_DEMO = auto(),
    MM_N64_US_GW = auto(),
    MM_N64_EU_1_0 = auto(),
    MM_N64_EU_1_1 = auto(),
    MM_N64_EU_1_1_VC = auto(),
    MM_N64_EU_1_1_DBG = auto(),
    MM_GC_JP = auto(),
    MM_GC_US = auto(),
    MM_GC_EU = auto(),

GAMEVERSION_ALL_OOT = (
    GameVersion.OOT_NTSC_0_9,
    GameVersion.OOT_NTSC_1_0,
    GameVersion.OOT_NTSC_1_1,
    GameVersion.OOT_NTSC_1_2,
    GameVersion.OOT_NTSC_1_2_GW,
    GameVersion.OOT_PAL_1_0,
    GameVersion.OOT_PAL_1_1,
    GameVersion.OOT_GC_JP,
    GameVersion.OOT_GC_JP_CE,
    GameVersion.OOT_GC_JP_MQ,
    GameVersion.OOT_GC_US,
    GameVersion.OOT_GC_US_MQ,
    GameVersion.OOT_GC_EU,
    GameVersion.OOT_GC_EU_DBG0,
    GameVersion.OOT_GC_EU_DBG1,
    GameVersion.OOT_GC_EU_MQ,
    GameVersion.OOT_GC_EU_MQ_DBG,
    GameVersion.OOT_IQUE_CN,
    GameVersion.OOT_IQUE_ZH,
)
GAMEVERSION_ALL_MM  = (
    GameVersion.MM_N64_JP_1_0,
    GameVersion.MM_N64_JP_1_1,
    GameVersion.MM_N64_US,
    GameVersion.MM_N64_US_DEMO,
    GameVersion.MM_N64_US_GW,
    GameVersion.MM_N64_EU_1_0,
    GameVersion.MM_N64_EU_1_1,
    GameVersion.MM_N64_EU_1_1_VC,
    GameVersion.MM_N64_EU_1_1_DBG,
    GameVersion.MM_GC_JP,
    GameVersion.MM_GC_US,
    GameVersion.MM_GC_EU,
)

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
    # OoT
  # "oot-ntsc-0.9"      :   GameVersionInfo(GameVersion.OOT_NTSC_0_9, MMLVersion.OOT,
    "oot-ntsc-1.0"      :   GameVersionInfo(GameVersion.OOT_NTSC_1_0, MMLVersion.OOT,
                                            0x0D390, 0x29DE0, 0x79470, 0xB896A0, 0xB89910, 0xB89AD0, 0xB8A1C0),
  # "oot-ntsc-1.1"      :   GameVersionInfo(GameVersion.OOT_NTSC_1_1, MMLVersion.OOT,
  # "oot-ntsc-1.2"      :   GameVersionInfo(GameVersion.OOT_NTSC_1_2, MMLVersion.OOT,
  # "oot-ntsc-1.2-gw"   :   GameVersionInfo(GameVersion.OOT_NTSC_1_2_GW, MMLVersion.OOT,
  # "oot-pal-1.0"       :   GameVersionInfo(GameVersion.OOT_PAL_1_0, MMLVersion.OOT,
  # "oot-pal-1.1"       :   GameVersionInfo(GameVersion.OOT_PAL_1_1, MMLVersion.OOT,
  # "oot-gc-jp"         :   GameVersionInfo(GameVersion.OOT_GC_JP, MMLVersion.OOT,
  # "oot-gc-jp-ce"      :   GameVersionInfo(GameVersion.OOT_GC_JP_CE, MMLVersion.OOT,
  # "oot-gc-jp-mq"      :   GameVersionInfo(GameVersion.OOT_GC_JP_MQ, MMLVersion.OOT,
    "oot-gc-us"         :   GameVersionInfo(GameVersion.OOT_GC_US, MMLVersion.OOT,
                                            0x0D0C0, 0x38E80, 0x88900, 0xB87DA0, 0xB88010, 0xB881D0, 0xB888C0),
  # "oot-gc-us-mq"      :   GameVersionInfo(GameVersion.OOT_GC_US_MQ, MMLVersion.OOT,
    "oot-gc-eu"         :   GameVersionInfo(GameVersion.OOT_GC_EU, MMLVersion.OOT,
                                            0x0D0D0, 0x38E90, 0x88910, 0xB87590, 0xB87800, 0xB879C0, 0xB880B0),
  # "oot-gc-eu-dbg0"    :   GameVersionInfo(GameVersion.OOT_GC_EU_DBG0, MMLVersion.OOT,
  # "oot-gc-eu-dbg1"    :   GameVersionInfo(GameVersion.OOT_GC_EU_DBG1, MMLVersion.OOT,
    "oot-gc-eu-mq"      :   GameVersionInfo(GameVersion.OOT_GC_EU_MQ, MMLVersion.OOT,
                                            0x0D0D0, 0x38E90, 0x88910, 0xB87570, 0xB877E0, 0xB879A0, 0xB88090),
    "oot-gc-eu-mq-dbg"  :   GameVersionInfo(GameVersion.OOT_GC_EU_MQ_DBG, MMLVersion.OOT,
                                            0x19030, 0x44DF0, 0x94870, 0xBCC270, 0xBCC4E0, 0xBCC6A0, 0xBCCD90),
  # "oot-ique-cn"       :   GameVersionInfo(GameVersion.OOT_IQUE_CN, MMLVersion.OOT,
  # "oot-ique-zh"       :   GameVersionInfo(GameVersion.OOT_IQUE_ZH, MMLVersion.OOT,
    # MM
    "mm-n64-jp-1.0"     :   GameVersionInfo(GameVersion.MM_N64_JP_1_0, MMLVersion.MM,
                                            0x222F0, 0x48160, 0x995D0, 0xC97130, 0xC973C0, 0xC975D0, 0xC97DE0),
  # "mm-n64-jp-1.1"         GameVersionInfo(GameVersion.MM_N64_JP_1_1, MMLVersion.MM,
    "mm-n64-us"         :   GameVersionInfo(GameVersion.MM_N64_US, MMLVersion.MM,
                                            0x20700, 0x46AF0, 0x97F70, 0xC776C0, 0xC77960, 0xC77B70, 0xC78380),
  # "mm-n64-us-demo"    :   GameVersionInfo(GameVersion.MM_N64_US_DEMO, MMLVersion.MM,
  # "mm-n64-us-gw"      :   GameVersionInfo(GameVersion.MM_N64_US_GW, MMLVersion.MM,
  # "mm-n64-eu-1.0"     :   GameVersionInfo(GameVersion.MM_N64_EU_1_0, MMLVersion.MM,
  # "mm-n64-eu-1.1"     :   GameVersionInfo(GameVersion.MM_N64_EU_1_1, MMLVersion.MM,
  # "mm-n64-eu-1.1-vc"  :   GameVersionInfo(GameVersion.MM_N64_EU_1_1_VC, MMLVersion.MM,
    "mm-n64-eu-1.1-dbg" :   GameVersionInfo(GameVersion.MM_N64_EU_1_1_DBG, MMLVersion.MM,
                                            0x2B2D0, 0x516C0, 0xA2B40, 0xE0F7E0, 0xE0FA80, 0xE0FC90, 0xE104A0),
  # "mm-gc-jp"          :   GameVersionInfo(GameVersion.MM_GC_JP, MMLVersion.MM,
  # "mm-gc-us"          :   GameVersionInfo(GameVersion.MM_GC_US, MMLVersion.MM,
  # "mm-gc-eu"          :   GameVersionInfo(GameVersion.MM_GC_EU, MMLVersion.MM,
}

# ======================================================================================================================
#   Hacks
# ======================================================================================================================

# Some sequences are "handwritten", we don't extract these by default as we want these checked in for documentation.
HANDWRITTEN_SEQUENCES_OOT = (0, 1, 2, 109)
HANDWRITTEN_SEQUENCES_MM  = (0, 1)

# Some bugged soundfonts report the wrong samplebank. Map them to the correct samplebank for proper sample discovery.
FAKE_BANKS = {
    # OoT
    GameVersion.OOT_NTSC_0_9     : { 37 : 2 },
    GameVersion.OOT_NTSC_1_0     : { 37 : 2 },
    GameVersion.OOT_NTSC_1_1     : { 37 : 2 },
    GameVersion.OOT_NTSC_1_2     : { 37 : 2 },
    GameVersion.OOT_NTSC_1_2_GW  : { 37 : 2 },
    GameVersion.OOT_PAL_1_0      : { 37 : 2 },
    GameVersion.OOT_PAL_1_1      : { 37 : 2 },
    GameVersion.OOT_GC_JP        : { 37 : 2 },
    GameVersion.OOT_GC_JP_CE     : { 37 : 2 },
    GameVersion.OOT_GC_JP_MQ     : { 37 : 2 },
    GameVersion.OOT_GC_US        : { 37 : 2 },
    GameVersion.OOT_GC_US_MQ     : { 37 : 2 },
    GameVersion.OOT_GC_EU        : { 37 : 2 },
    GameVersion.OOT_GC_EU_DBG0   : { 37 : 2 },
    GameVersion.OOT_GC_EU_DBG1   : { 37 : 2 },
    GameVersion.OOT_GC_EU_MQ     : { 37 : 2 },
    GameVersion.OOT_GC_EU_MQ_DBG : { 37 : 2 },
    GameVersion.OOT_IQUE_CN      : { 37 : 2 },
    GameVersion.OOT_IQUE_ZH      : { 37 : 2 },
    # MM
    GameVersion.MM_N64_JP_1_0     : { 39 : 2 },
    GameVersion.MM_N64_JP_1_1     : { 40 : 2 },
    GameVersion.MM_N64_US         : { 40 : 2 },
    GameVersion.MM_N64_US_DEMO    : { 40 : 2 },
    GameVersion.MM_N64_US_GW      : { 40 : 2 },
    GameVersion.MM_N64_EU_1_0     : { 40 : 2 },
    GameVersion.MM_N64_EU_1_1     : { 40 : 2 },
    GameVersion.MM_N64_EU_1_1_VC  : { 40 : 2 },
    GameVersion.MM_N64_EU_1_1_DBG : { 40 : 2 },
    GameVersion.MM_GC_JP          : { 40 : 2 },
    GameVersion.MM_GC_US          : { 40 : 2 },
    GameVersion.MM_GC_EU          : { 40 : 2 },
}


# Some audiotable banks have a buffer clearing bug. Indicate which banks suffer from this.
AUDIOTABLE_BUFFER_BUGS = {
    # OoT
    GameVersion.OOT_NTSC_0_9     : (0,),
    GameVersion.OOT_NTSC_1_0     : (0,),
    GameVersion.OOT_NTSC_1_1     : (0,),
    GameVersion.OOT_NTSC_1_2     : (0,),
    GameVersion.OOT_NTSC_1_2_GW  : (0,),
    GameVersion.OOT_PAL_1_0      : (0,),
    GameVersion.OOT_PAL_1_1      : (0,),
    GameVersion.OOT_GC_JP        : (0,),
    GameVersion.OOT_GC_JP_CE     : (0,),
    GameVersion.OOT_GC_JP_MQ     : (0,),
    GameVersion.OOT_GC_US        : (0,),
    GameVersion.OOT_GC_US_MQ     : (0,),
    GameVersion.OOT_GC_EU        : (0,),
    GameVersion.OOT_GC_EU_DBG0   : (0,),
    GameVersion.OOT_GC_EU_DBG1   : (0,),
    GameVersion.OOT_GC_EU_MQ     : (0,),
    GameVersion.OOT_GC_EU_MQ_DBG : (0,),
    GameVersion.OOT_IQUE_CN      : (0,),
    GameVersion.OOT_IQUE_ZH      : (0,),
    # MM
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
    # OoT
    GameVersion.OOT_NTSC_0_9     : SEQ_DISAS_HACKS_OOT,
    GameVersion.OOT_NTSC_1_0     : SEQ_DISAS_HACKS_OOT,
    GameVersion.OOT_NTSC_1_1     : SEQ_DISAS_HACKS_OOT,
    GameVersion.OOT_NTSC_1_2     : SEQ_DISAS_HACKS_OOT,
    GameVersion.OOT_NTSC_1_2_GW  : SEQ_DISAS_HACKS_OOT,
    GameVersion.OOT_PAL_1_0      : SEQ_DISAS_HACKS_OOT,
    GameVersion.OOT_PAL_1_1      : SEQ_DISAS_HACKS_OOT,
    GameVersion.OOT_GC_JP        : SEQ_DISAS_HACKS_OOT,
    GameVersion.OOT_GC_JP_CE     : SEQ_DISAS_HACKS_OOT,
    GameVersion.OOT_GC_JP_MQ     : SEQ_DISAS_HACKS_OOT,
    GameVersion.OOT_GC_US        : SEQ_DISAS_HACKS_OOT,
    GameVersion.OOT_GC_US_MQ     : SEQ_DISAS_HACKS_OOT,
    GameVersion.OOT_GC_EU        : SEQ_DISAS_HACKS_OOT,
    GameVersion.OOT_GC_EU_DBG0   : SEQ_DISAS_HACKS_OOT,
    GameVersion.OOT_GC_EU_DBG1   : SEQ_DISAS_HACKS_OOT,
    GameVersion.OOT_GC_EU_MQ     : SEQ_DISAS_HACKS_OOT,
    GameVersion.OOT_GC_EU_MQ_DBG : SEQ_DISAS_HACKS_OOT,
    GameVersion.OOT_IQUE_CN      : SEQ_DISAS_HACKS_OOT,
    GameVersion.OOT_IQUE_ZH      : SEQ_DISAS_HACKS_OOT,
    # MM
    GameVersion.MM_N64_JP_1_0     : SEQ_DISAS_HACKS_MM,
    GameVersion.MM_N64_JP_1_1     : SEQ_DISAS_HACKS_MM,
    GameVersion.MM_N64_US         : SEQ_DISAS_HACKS_MM,
    GameVersion.MM_N64_US_DEMO    : SEQ_DISAS_HACKS_MM,
    GameVersion.MM_N64_US_GW      : SEQ_DISAS_HACKS_MM,
    GameVersion.MM_N64_EU_1_0     : SEQ_DISAS_HACKS_MM,
    GameVersion.MM_N64_EU_1_1     : SEQ_DISAS_HACKS_MM,
    GameVersion.MM_N64_EU_1_1_VC  : SEQ_DISAS_HACKS_MM,
    GameVersion.MM_N64_EU_1_1_DBG : SEQ_DISAS_HACKS_MM,
    GameVersion.MM_GC_JP          : SEQ_DISAS_HACKS_MM,
    GameVersion.MM_GC_US          : SEQ_DISAS_HACKS_MM,
    GameVersion.MM_GC_EU          : SEQ_DISAS_HACKS_MM,
}
