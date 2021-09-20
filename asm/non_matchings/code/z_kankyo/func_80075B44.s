.rdata
glabel D_8013C738
    .asciz "\n\n\nNa_StartMorinigBgm\n\n"
    .balign 4

.late_rodata
glabel jtbl_8013C80C
   .word L80075B84
   .word L80075BD8
   .word L80075C30
   .word L80075C6C
   .word L80075CB4
   .word L80075CE0
   .word L80075D30
   .word L80075DE4
   .word L80075E34

.text
glabel func_80075B44
/* AECCE4 80075B44 27BDFFE0 */  addiu $sp, $sp, -0x20
/* AECCE8 80075B48 AFB00014 */  sw    $s0, 0x14($sp)
/* AECCEC 80075B4C 3C010001 */  lui   $at, 1
/* AECCF0 80075B50 AFBF001C */  sw    $ra, 0x1c($sp)
/* AECCF4 80075B54 AFB10018 */  sw    $s1, 0x18($sp)
/* AECCF8 80075B58 00818021 */  addu  $s0, $a0, $at
/* AECCFC 80075B5C 92020B04 */  lbu   $v0, 0xb04($s0)
/* AECD00 80075B60 00808825 */  move  $s1, $a0
/* AECD04 80075B64 2C410009 */  sltiu $at, $v0, 9
/* AECD08 80075B68 102000BA */  beqz  $at, .L80075E54
/* AECD0C 80075B6C 00027080 */   sll   $t6, $v0, 2
/* AECD10 80075B70 3C018014 */  lui   $at, %hi(jtbl_8013C80C)
/* AECD14 80075B74 002E0821 */  addu  $at, $at, $t6
/* AECD18 80075B78 8C2EC80C */  lw    $t6, %lo(jtbl_8013C80C)($at)
/* AECD1C 80075B7C 01C00008 */  jr    $t6
/* AECD20 80075B80 00000000 */   nop   
glabel L80075B84
/* AECD24 80075B84 24040056 */  li    $a0, 86
/* AECD28 80075B88 24050001 */  li    $a1, 1
/* AECD2C 80075B8C 0C03DB56 */  jal   func_800F6D58
/* AECD30 80075B90 00003025 */   move  $a2, $zero
/* AECD34 80075B94 920F0B12 */  lbu   $t7, 0xb12($s0)
/* AECD38 80075B98 55E0000A */  bnezl $t7, .L80075BC4
/* AECD3C 80075B9C 92190B04 */   lbu   $t9, 0xb04($s0)
/* AECD40 80075BA0 92180B16 */  lbu   $t8, 0xb16($s0)
/* AECD44 80075BA4 3C048014 */  lui   $a0, %hi(D_8013C738) # $a0, 0x8014
/* AECD48 80075BA8 57000006 */  bnezl $t8, .L80075BC4
/* AECD4C 80075BAC 92190B04 */   lbu   $t9, 0xb04($s0)
/* AECD50 80075BB0 0C00084C */  jal   osSyncPrintf
/* AECD54 80075BB4 2484C738 */   addiu $a0, %lo(D_8013C738) # addiu $a0, $a0, -0x38c8
/* AECD58 80075BB8 0C03D544 */  jal   func_800F5510
/* AECD5C 80075BBC 922407A4 */   lbu   $a0, 0x7a4($s1)
/* AECD60 80075BC0 92190B04 */  lbu   $t9, 0xb04($s0)
.L80075BC4:
/* AECD64 80075BC4 3C010001 */  lui   $at, 1
/* AECD68 80075BC8 00310821 */  addu  $at, $at, $s1
/* AECD6C 80075BCC 27280001 */  addiu $t0, $t9, 1
/* AECD70 80075BD0 100000A0 */  b     .L80075E54
/* AECD74 80075BD4 A0280B04 */   sb    $t0, 0xb04($at)
glabel L80075BD8
/* AECD78 80075BD8 3C038016 */  lui   $v1, %hi(gSaveContext) # $v1, 0x8016
/* AECD7C 80075BDC 2463E660 */  addiu $v1, %lo(gSaveContext) # addiu $v1, $v1, -0x19a0
/* AECD80 80075BE0 9469000C */  lhu   $t1, 0xc($v1)
/* AECD84 80075BE4 3401B71D */  li    $at, 46877
/* AECD88 80075BE8 0121082A */  slt   $at, $t1, $at
/* AECD8C 80075BEC 5420009A */  bnezl $at, .L80075E58
/* AECD90 80075BF0 8FBF001C */   lw    $ra, 0x1c($sp)
/* AECD94 80075BF4 920A0B12 */  lbu   $t2, 0xb12($s0)
/* AECD98 80075BF8 55400009 */  bnezl $t2, .L80075C20
/* AECD9C 80075BFC 3C010001 */   lui   $at, 1
/* AECDA0 80075C00 920B0B16 */  lbu   $t3, 0xb16($s0)
/* AECDA4 80075C04 3C0410F0 */  lui   $a0, (0x10F000FF >> 16) # lui $a0, 0x10f0
/* AECDA8 80075C08 55600005 */  bnezl $t3, .L80075C20
/* AECDAC 80075C0C 3C010001 */   lui   $at, 1
/* AECDB0 80075C10 0C03E803 */  jal   Audio_QueueSeqCmd
/* AECDB4 80075C14 348400FF */   ori   $a0, (0x10F000FF & 0xFFFF) # ori $a0, $a0, 0xff
/* AECDB8 80075C18 92020B04 */  lbu   $v0, 0xb04($s0)
/* AECDBC 80075C1C 3C010001 */  lui   $at, 1
.L80075C20:
/* AECDC0 80075C20 00310821 */  addu  $at, $at, $s1
/* AECDC4 80075C24 244C0001 */  addiu $t4, $v0, 1
/* AECDC8 80075C28 1000008A */  b     .L80075E54
/* AECDCC 80075C2C A02C0B04 */   sb    $t4, 0xb04($at)
glabel L80075C30
/* AECDD0 80075C30 3C038016 */  lui   $v1, %hi(gSaveContext) # $v1, 0x8016
/* AECDD4 80075C34 2463E660 */  addiu $v1, %lo(gSaveContext) # addiu $v1, $v1, -0x19a0
/* AECDD8 80075C38 946D000C */  lhu   $t5, 0xc($v1)
/* AECDDC 80075C3C 3401C001 */  li    $at, 49153
/* AECDE0 80075C40 01A1082A */  slt   $at, $t5, $at
/* AECDE4 80075C44 54200084 */  bnezl $at, .L80075E58
/* AECDE8 80075C48 8FBF001C */   lw    $ra, 0x1c($sp)
/* AECDEC 80075C4C 0C01E233 */  jal   func_800788CC
/* AECDF0 80075C50 240428AE */   li    $a0, 10414
/* AECDF4 80075C54 920E0B04 */  lbu   $t6, 0xb04($s0)
/* AECDF8 80075C58 3C010001 */  lui   $at, 1
/* AECDFC 80075C5C 00310821 */  addu  $at, $at, $s1
/* AECE00 80075C60 25CF0001 */  addiu $t7, $t6, 1
/* AECE04 80075C64 1000007B */  b     .L80075E54
/* AECE08 80075C68 A02F0B04 */   sb    $t7, 0xb04($at)
glabel L80075C6C
/* AECE0C 80075C6C 92180B12 */  lbu   $t8, 0xb12($s0)
/* AECE10 80075C70 5700000C */  bnezl $t8, .L80075CA4
/* AECE14 80075C74 3C010001 */   lui   $at, 1
/* AECE18 80075C78 92190B16 */  lbu   $t9, 0xb16($s0)
/* AECE1C 80075C7C 57200009 */  bnezl $t9, .L80075CA4
/* AECE20 80075C80 3C010001 */   lui   $at, 1
/* AECE24 80075C84 0C03DBED */  jal   func_800F6FB4
/* AECE28 80075C88 922407A5 */   lbu   $a0, 0x7a5($s1)
/* AECE2C 80075C8C 24040001 */  li    $a0, 1
/* AECE30 80075C90 24050001 */  li    $a1, 1
/* AECE34 80075C94 0C03DB56 */  jal   func_800F6D58
/* AECE38 80075C98 24060001 */   li    $a2, 1
/* AECE3C 80075C9C 92020B04 */  lbu   $v0, 0xb04($s0)
/* AECE40 80075CA0 3C010001 */  lui   $at, 1
.L80075CA4:
/* AECE44 80075CA4 00310821 */  addu  $at, $at, $s1
/* AECE48 80075CA8 24480001 */  addiu $t0, $v0, 1
/* AECE4C 80075CAC 10000069 */  b     .L80075E54
/* AECE50 80075CB0 A0280B04 */   sb    $t0, 0xb04($at)
glabel L80075CB4
/* AECE54 80075CB4 3C038016 */  lui   $v1, %hi(gSaveContext) # $v1, 0x8016
/* AECE58 80075CB8 2463E660 */  addiu $v1, %lo(gSaveContext) # addiu $v1, $v1, -0x19a0
/* AECE5C 80075CBC 9469000C */  lhu   $t1, 0xc($v1)
/* AECE60 80075CC0 3401CAAC */  li    $at, 51884
/* AECE64 80075CC4 244A0001 */  addiu $t2, $v0, 1
/* AECE68 80075CC8 0121082A */  slt   $at, $t1, $at
/* AECE6C 80075CCC 14200061 */  bnez  $at, .L80075E54
/* AECE70 80075CD0 3C010001 */   lui   $at, 1
/* AECE74 80075CD4 00310821 */  addu  $at, $at, $s1
/* AECE78 80075CD8 1000005E */  b     .L80075E54
/* AECE7C 80075CDC A02A0B04 */   sb    $t2, 0xb04($at)
glabel L80075CE0
/* AECE80 80075CE0 24040001 */  li    $a0, 1
/* AECE84 80075CE4 24050001 */  li    $a1, 1
/* AECE88 80075CE8 0C03DB56 */  jal   func_800F6D58
/* AECE8C 80075CEC 00003025 */   move  $a2, $zero
/* AECE90 80075CF0 920B0B12 */  lbu   $t3, 0xb12($s0)
/* AECE94 80075CF4 55600009 */  bnezl $t3, .L80075D1C
/* AECE98 80075CF8 920D0B04 */   lbu   $t5, 0xb04($s0)
/* AECE9C 80075CFC 920C0B16 */  lbu   $t4, 0xb16($s0)
/* AECEA0 80075D00 24040024 */  li    $a0, 36
/* AECEA4 80075D04 24050001 */  li    $a1, 1
/* AECEA8 80075D08 55800004 */  bnezl $t4, .L80075D1C
/* AECEAC 80075D0C 920D0B04 */   lbu   $t5, 0xb04($s0)
/* AECEB0 80075D10 0C03DB56 */  jal   func_800F6D58
/* AECEB4 80075D14 24060001 */   li    $a2, 1
/* AECEB8 80075D18 920D0B04 */  lbu   $t5, 0xb04($s0)
.L80075D1C:
/* AECEBC 80075D1C 3C010001 */  lui   $at, 1
/* AECEC0 80075D20 00310821 */  addu  $at, $at, $s1
/* AECEC4 80075D24 25AE0001 */  addiu $t6, $t5, 1
/* AECEC8 80075D28 1000004A */  b     .L80075E54
/* AECECC 80075D2C A02E0B04 */   sb    $t6, 0xb04($at)
glabel L80075D30
/* AECED0 80075D30 3C038016 */  lui   $v1, %hi(gSaveContext) # $v1, 0x8016
/* AECED4 80075D34 2463E660 */  addiu $v1, %lo(gSaveContext) # addiu $v1, $v1, -0x19a0
/* AECED8 80075D38 9462000C */  lhu   $v0, 0xc($v1)
/* AECEDC 80075D3C 3401CAAC */  li    $at, 51884
/* AECEE0 80075D40 0041082A */  slt   $at, $v0, $at
/* AECEE4 80075D44 10200043 */  beqz  $at, .L80075E54
/* AECEE8 80075D48 28414556 */   slti  $at, $v0, 0x4556
/* AECEEC 80075D4C 14200041 */  bnez  $at, .L80075E54
/* AECEF0 80075D50 24090001 */   li    $t1, 1
/* AECEF4 80075D54 8C6F0014 */  lw    $t7, 0x14($v1)
/* AECEF8 80075D58 8C790018 */  lw    $t9, 0x18($v1)
/* AECEFC 80075D5C A069141C */  sb    $t1, 0x141c($v1)
/* AECF00 80075D60 25F80001 */  addiu $t8, $t7, 1
/* AECF04 80075D64 27280001 */  addiu $t0, $t9, 1
/* AECF08 80075D68 AC780014 */  sw    $t8, 0x14($v1)
/* AECF0C 80075D6C AC680018 */  sw    $t0, 0x18($v1)
/* AECF10 80075D70 0C01E221 */  jal   func_80078884
/* AECF14 80075D74 24042813 */   li    $a0, 10259
/* AECF18 80075D78 02202025 */  move  $a0, $s1
/* AECF1C 80075D7C 24050021 */  li    $a1, 33
/* AECF20 80075D80 0C021A79 */  jal   Inventory_ReplaceItem
/* AECF24 80075D84 24060022 */   li    $a2, 34
/* AECF28 80075D88 14400006 */  bnez  $v0, .L80075DA4
/* AECF2C 80075D8C 02202025 */   move  $a0, $s1
/* AECF30 80075D90 2405002D */  li    $a1, 45
/* AECF34 80075D94 0C021A79 */  jal   Inventory_ReplaceItem
/* AECF38 80075D98 2406002E */   li    $a2, 46
/* AECF3C 80075D9C 5040000C */  beql  $v0, $zero, .L80075DD0
/* AECF40 80075DA0 920B0B04 */   lbu   $t3, 0xb04($s0)
.L80075DA4:
/* AECF44 80075DA4 922A1D6C */  lbu   $t2, 0x1d6c($s1)
/* AECF48 80075DA8 55400009 */  bnezl $t2, .L80075DD0
/* AECF4C 80075DAC 920B0B04 */   lbu   $t3, 0xb04($s0)
/* AECF50 80075DB0 0C023A62 */  jal   Player_InCsMode
/* AECF54 80075DB4 02202025 */   move  $a0, $s1
/* AECF58 80075DB8 14400004 */  bnez  $v0, .L80075DCC
/* AECF5C 80075DBC 02202025 */   move  $a0, $s1
/* AECF60 80075DC0 24053066 */  li    $a1, 12390
/* AECF64 80075DC4 0C042DA0 */  jal   Message_StartTextbox
/* AECF68 80075DC8 00003025 */   move  $a2, $zero
.L80075DCC:
/* AECF6C 80075DCC 920B0B04 */  lbu   $t3, 0xb04($s0)
.L80075DD0:
/* AECF70 80075DD0 3C010001 */  lui   $at, 1
/* AECF74 80075DD4 00310821 */  addu  $at, $at, $s1
/* AECF78 80075DD8 256C0001 */  addiu $t4, $t3, 1
/* AECF7C 80075DDC 1000001D */  b     .L80075E54
/* AECF80 80075DE0 A02C0B04 */   sb    $t4, 0xb04($at)
glabel L80075DE4
/* AECF84 80075DE4 24040024 */  li    $a0, 36
/* AECF88 80075DE8 24050001 */  li    $a1, 1
/* AECF8C 80075DEC 0C03DB56 */  jal   func_800F6D58
/* AECF90 80075DF0 00003025 */   move  $a2, $zero
/* AECF94 80075DF4 920D0B12 */  lbu   $t5, 0xb12($s0)
/* AECF98 80075DF8 55A00009 */  bnezl $t5, .L80075E20
/* AECF9C 80075DFC 920F0B04 */   lbu   $t7, 0xb04($s0)
/* AECFA0 80075E00 920E0B16 */  lbu   $t6, 0xb16($s0)
/* AECFA4 80075E04 24040056 */  li    $a0, 86
/* AECFA8 80075E08 24050001 */  li    $a1, 1
/* AECFAC 80075E0C 55C00004 */  bnezl $t6, .L80075E20
/* AECFB0 80075E10 920F0B04 */   lbu   $t7, 0xb04($s0)
/* AECFB4 80075E14 0C03DB56 */  jal   func_800F6D58
/* AECFB8 80075E18 24060001 */   li    $a2, 1
/* AECFBC 80075E1C 920F0B04 */  lbu   $t7, 0xb04($s0)
.L80075E20:
/* AECFC0 80075E20 3C010001 */  lui   $at, 1
/* AECFC4 80075E24 00310821 */  addu  $at, $at, $s1
/* AECFC8 80075E28 25F80001 */  addiu $t8, $t7, 1
/* AECFCC 80075E2C 10000009 */  b     .L80075E54
/* AECFD0 80075E30 A0380B04 */   sb    $t8, 0xb04($at)
glabel L80075E34
/* AECFD4 80075E34 3C038016 */  lui   $v1, %hi(gSaveContext) # $v1, 0x8016
/* AECFD8 80075E38 2463E660 */  addiu $v1, %lo(gSaveContext) # addiu $v1, $v1, -0x19a0
/* AECFDC 80075E3C 9479000C */  lhu   $t9, 0xc($v1)
/* AECFE0 80075E40 2B214AAC */  slti  $at, $t9, 0x4aac
/* AECFE4 80075E44 14200003 */  bnez  $at, .L80075E54
/* AECFE8 80075E48 3C010001 */   lui   $at, 1
/* AECFEC 80075E4C 00310821 */  addu  $at, $at, $s1
/* AECFF0 80075E50 A0200B04 */  sb    $zero, 0xb04($at)
.L80075E54:
/* AECFF4 80075E54 8FBF001C */  lw    $ra, 0x1c($sp)
.L80075E58:
/* AECFF8 80075E58 8FB00014 */  lw    $s0, 0x14($sp)
/* AECFFC 80075E5C 8FB10018 */  lw    $s1, 0x18($sp)
/* AED000 80075E60 03E00008 */  jr    $ra
/* AED004 80075E64 27BD0020 */   addiu $sp, $sp, 0x20

