.include "macro.inc"

# assembler directives
.set noat      # allow manual use of $at
.set noreorder # don't insert nops after branches
.set gp=64     # allow use of 64-bit general purpose registers

.section .rodata

.balign 16

glabel D_80155340
    .word 0x00DC00DF, 0x00E100E3, 0x00E500E7, 0x00E900EB, 0x00ED00EF, 0x00F100F3, 0x00F500F7, 0x00F900FB 
    .word 0x00FD00FF, 0x01010103, 0x01050107, 0x0109010B, 0x010D010F, 0x01110113, 0x01150117, 0x0119011B 
    .word 0x011D011F, 0x01210123, 0x01250127, 0x0129012B, 0x012D012F, 0x01310133, 0x01350137, 0x0139013B 
    .word 0x013D013F, 0x01410143, 0x01450147, 0x0149014B, 0x014D014F, 0x01510153, 0x01550157, 0x0159015B 
    .word 0x015D015F, 0x01610163, 0x01650167, 0x0169016B, 0x016D016F, 0x01710173, 0x01750177, 0x0179017B 
    .word 0x017D017F, 0x01810183, 0x01850187, 0x0189018B, 0x018D018F, 0x01910193, 0x01950197, 0x0199019B 
    .word 0x019D019F, 0x01A101A3, 0x01A501A7, 0x01A901AB, 0x01AD01AF, 0x01B101B3, 0x01B501B7, 0x02010001 
    .word 0x02010301, 0x03010301, 0x03010301, 0x03010301, 0x03010301, 0x03010301, 0x03010301, 0x03010301 
    .word 0x03010301, 0x03010301, 0x03010301, 0x03010B01, 0x03010301, 0x03010401, 0x05010601, 0x03012301 
    .word 0x03012301, 0x03012301, 0x12010701, 0x08010901, 0x09010A01, 0x03010C01, 0x03011E01, 0x0D010E01 
    .word 0x03010301, 0x12011201, 0x12011201, 0x12010301, 0x23010901, 0x03010F01, 0x09010501, 0x10011101 
    .word 0x11011101, 0x03010001, 0x00010001, 0x00010001, 0x00010301, 0x09010801, 0x13011401, 0x09011501 
    .word 0x03010301, 0x03010301, 0x16011301, 0x09011701, 0x12012401, 0x18011901, 0x13012001, 0x1B011C01 
    .word 0x1D010301, 0x1F012001, 0x20010901, 0x21012201, 0x21010901, 0x20010302, 0x01000000, 0x00000000

glabel gSequenceTable
    .half 110
    .half 0x0000
    .word 0x00000000, 0x00000000, 0x00000000

    .word AudioSeq_0_Start,     AudioSeq_0_Length
    .byte 0x02, 0x00
    .fill 6
    .word AudioSeq_1_Start,     AudioSeq_1_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_2_Start,     AudioSeq_2_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_3_Start,     AudioSeq_3_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_4_Start,     AudioSeq_4_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_5_Start,     AudioSeq_5_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_6_Start,     AudioSeq_6_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_7_Start,     AudioSeq_7_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_8_Start,     AudioSeq_8_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_9_Start,     AudioSeq_9_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_10_Start,    AudioSeq_10_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_11_Start,    AudioSeq_11_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_12_Start,    AudioSeq_12_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_13_Start,    AudioSeq_13_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_14_Start,    AudioSeq_14_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_15_Start,    AudioSeq_15_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_16_Start,    AudioSeq_16_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_17_Start,    AudioSeq_17_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_18_Start,    AudioSeq_18_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_19_Start,    AudioSeq_19_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_20_Start,    AudioSeq_20_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_21_Start,    AudioSeq_21_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_22_Start,    AudioSeq_22_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_23_Start,    AudioSeq_23_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_24_Start,    AudioSeq_24_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_25_Start,    AudioSeq_25_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_26_Start,    AudioSeq_26_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_27_Start,    AudioSeq_27_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_28_Start,    AudioSeq_28_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_29_Start,    AudioSeq_29_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_30_Start,    AudioSeq_30_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_31_Start,    AudioSeq_31_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_32_Start,    AudioSeq_32_Length
    .byte 0x02, 0x01
    .fill 6
    .word AudioSeq_33_Start,    AudioSeq_33_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_34_Start,    AudioSeq_34_Length
    .byte 0x02, 0x01
    .fill 6
    .word AudioSeq_35_Start,    AudioSeq_35_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_36_Start,    AudioSeq_36_Length
    .byte 0x02, 0x01
    .fill 6
    .word AudioSeq_37_Start,    AudioSeq_37_Length
    .byte 0x02, 0x01
    .fill 6
    .word AudioSeq_38_Start,    AudioSeq_38_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_39_Start,    AudioSeq_39_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_40_Start,    AudioSeq_40_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_41_Start,    AudioSeq_41_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_42_Start,    AudioSeq_42_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_43_Start,    AudioSeq_43_Length
    .byte 0x02, 0x01
    .fill 6
    .word AudioSeq_44_Start,    AudioSeq_44_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_45_Start,    AudioSeq_45_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_46_Start,    AudioSeq_46_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_47_Start,    AudioSeq_47_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_48_Start,    AudioSeq_48_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_49_Start,    AudioSeq_49_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_50_Start,    AudioSeq_50_Length
    .byte 0x02, 0x01
    .fill 6
    .word AudioSeq_51_Start,    AudioSeq_51_Length
    .byte 0x02, 0x01
    .fill 6
    .word AudioSeq_52_Start,    AudioSeq_52_Length
    .byte 0x02, 0x01
    .fill 6
    .word AudioSeq_53_Start,    AudioSeq_53_Length
    .byte 0x02, 0x01
    .fill 6
    .word AudioSeq_54_Start,    AudioSeq_54_Length
    .byte 0x02, 0x01
    .fill 6
    .word AudioSeq_55_Start,    AudioSeq_55_Length
    .byte 0x02, 0x01
    .fill 6
    .word AudioSeq_56_Start,    AudioSeq_56_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_57_Start,    AudioSeq_57_Length
    .byte 0x02, 0x01
    .fill 6
    .word AudioSeq_58_Start,    AudioSeq_58_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_59_Start,    AudioSeq_59_Length
    .byte 0x02, 0x01
    .fill 6
    .word AudioSeq_60_Start,    AudioSeq_60_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_61_Start,    AudioSeq_61_Length
    .byte 0x02, 0x01
    .fill 6
    .word AudioSeq_62_Start,    AudioSeq_62_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_63_Start,    AudioSeq_63_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_64_Start,    AudioSeq_64_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_65_Start,    AudioSeq_65_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_66_Start,    AudioSeq_66_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_67_Start,    AudioSeq_67_Length
    .byte 0x02, 0x01
    .fill 6
    .word AudioSeq_68_Start,    AudioSeq_68_Length
    .byte 0x02, 0x01
    .fill 6
    .word AudioSeq_69_Start,    AudioSeq_69_Length
    .byte 0x02, 0x01
    .fill 6
    .word AudioSeq_70_Start,    AudioSeq_70_Length
    .byte 0x02, 0x01
    .fill 6
    .word AudioSeq_71_Start,    AudioSeq_71_Length
    .byte 0x02, 0x01
    .fill 6
    .word AudioSeq_72_Start,    AudioSeq_72_Length
    .byte 0x02, 0x01
    .fill 6
    .word AudioSeq_73_Start,    AudioSeq_73_Length
    .byte 0x02, 0x01
    .fill 6
    .word AudioSeq_74_Start,    AudioSeq_74_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_75_Start,    AudioSeq_75_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_76_Start,    AudioSeq_76_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_77_Start,    AudioSeq_77_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_78_Start,    AudioSeq_78_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_79_Start,    AudioSeq_79_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_80_Start,    AudioSeq_80_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_81_Start,    AudioSeq_81_Length
    .byte 0x02, 0x01
    .fill 6
    .word AudioSeq_82_Start,    AudioSeq_82_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_83_Start,    AudioSeq_83_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_84_Start,    AudioSeq_84_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_85_Start,    AudioSeq_85_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_86_Start,    AudioSeq_86_Length
    .byte 0x02, 0x02
    .fill 6
    .word 0x00000028,           AudioSeq_87_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_88_Start,    AudioSeq_88_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_89_Start,    AudioSeq_89_Length
    .byte 0x02, 0x01
    .fill 6
    .word AudioSeq_90_Start,    AudioSeq_90_Length
    .byte 0x02, 0x01
    .fill 6
    .word AudioSeq_91_Start,    AudioSeq_91_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_92_Start,    AudioSeq_92_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_93_Start,    AudioSeq_93_Length
    .byte 0x02, 0x01
    .fill 6
    .word AudioSeq_94_Start,    AudioSeq_94_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_95_Start,    AudioSeq_95_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_96_Start,    AudioSeq_96_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_97_Start,    AudioSeq_97_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_98_Start,    AudioSeq_98_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_99_Start,    AudioSeq_99_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_100_Start,   AudioSeq_100_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_101_Start,   AudioSeq_101_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_102_Start,   AudioSeq_102_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_103_Start,   AudioSeq_103_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_104_Start,   AudioSeq_104_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_105_Start,   AudioSeq_105_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_106_Start,   AudioSeq_106_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_107_Start,   AudioSeq_107_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_108_Start,   AudioSeq_108_Length
    .byte 0x02, 0x02
    .fill 6
    .word AudioSeq_109_Start,   AudioSeq_109_Length
    .byte 0x02, 0x02
    .fill 6
