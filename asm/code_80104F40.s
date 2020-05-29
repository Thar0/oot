.include "macro.inc"

# assembler directives
.set noat      # allow manual use of $at
.set noreorder # don't insert nops after branches
.set gp=64     # allow use of 64-bit general purpose registers

.section .text

.balign 16

glabel func_80104F40
/* B7C0E0 80104F40 27BDFF90 */  addiu $sp, $sp, -0x70
/* B7C0E4 80104F44 AFBF001C */  sw    $ra, 0x1c($sp)
/* B7C0E8 80104F48 AFB00018 */  sw    $s0, 0x18($sp)
/* B7C0EC 80104F4C 00A08025 */  move  $s0, $a1
/* B7C0F0 80104F50 AFA40070 */  sw    $a0, 0x70($sp)
/* B7C0F4 80104F54 0C0402E8 */  jal   __osSiGetAccess
/* B7C0F8 80104F58 AFA60078 */   sw    $a2, 0x78($sp)
/* B7C0FC 80104F5C 8FA40070 */  lw    $a0, 0x70($sp)
/* B7C100 80104F60 0C040644 */  jal   func_80101910
/* B7C104 80104F64 8FA50078 */   lw    $a1, 0x78($sp)
/* B7C108 80104F68 0C0402F9 */  jal   __osSiRelAccess
/* B7C10C 80104F6C AFA2006C */   sw    $v0, 0x6c($sp)
/* B7C110 80104F70 8FA3006C */  lw    $v1, 0x6c($sp)
/* B7C114 80104F74 8FAE0070 */  lw    $t6, 0x70($sp)
/* B7C118 80104F78 02002025 */  move  $a0, $s0
/* B7C11C 80104F7C 50600004 */  beql  $v1, $zero, .L80104F90
/* B7C120 80104F80 AE0E0004 */   sw    $t6, 4($s0)
/* B7C124 80104F84 1000006B */  b     .L80105134
/* B7C128 80104F88 00601025 */   move  $v0, $v1
/* B7C12C 80104F8C AE0E0004 */  sw    $t6, 4($s0)
.L80104F90:
/* B7C130 80104F90 8FAF0078 */  lw    $t7, 0x78($sp)
/* B7C134 80104F94 AE000000 */  sw    $zero, ($s0)
/* B7C138 80104F98 0C041452 */  jal   func_80105148
/* B7C13C 80104F9C AE0F0008 */   sw    $t7, 8($s0)
/* B7C140 80104FA0 10400003 */  beqz  $v0, .L80104FB0
/* B7C144 80104FA4 02002025 */   move  $a0, $s0
/* B7C148 80104FA8 10000063 */  b     .L80105138
/* B7C14C 80104FAC 8FBF001C */   lw    $ra, 0x1c($sp)
.L80104FB0:
/* B7C150 80104FB0 0C041320 */  jal   func_80104C80
/* B7C154 80104FB4 00002825 */   move  $a1, $zero
/* B7C158 80104FB8 10400003 */  beqz  $v0, .L80104FC8
/* B7C15C 80104FBC 24060001 */   li    $a2, 1
/* B7C160 80104FC0 1000005D */  b     .L80105138
/* B7C164 80104FC4 8FBF001C */   lw    $ra, 0x1c($sp)
.L80104FC8:
/* B7C168 80104FC8 8E040004 */  lw    $a0, 4($s0)
/* B7C16C 80104FCC 8E050008 */  lw    $a1, 8($s0)
/* B7C170 80104FD0 0C0417D0 */  jal   osReadMempak
/* B7C174 80104FD4 27A70048 */   addiu $a3, $sp, 0x48
/* B7C178 80104FD8 10400003 */  beqz  $v0, .L80104FE8
/* B7C17C 80104FDC 27A40048 */   addiu $a0, $sp, 0x48
/* B7C180 80104FE0 10000055 */  b     .L80105138
/* B7C184 80104FE4 8FBF001C */   lw    $ra, 0x1c($sp)
.L80104FE8:
/* B7C188 80104FE8 27A5006A */  addiu $a1, $sp, 0x6a
/* B7C18C 80104FEC 0C040925 */  jal   func_80102494
/* B7C190 80104FF0 27A60068 */   addiu $a2, $sp, 0x68
/* B7C194 80104FF4 97B8006A */  lhu   $t8, 0x6a($sp)
/* B7C198 80104FF8 97B90064 */  lhu   $t9, 0x64($sp)
/* B7C19C 80104FFC 27A50048 */  addiu $a1, $sp, 0x48
/* B7C1A0 80105000 AFA50044 */  sw    $a1, 0x44($sp)
/* B7C1A4 80105004 17190004 */  bne   $t8, $t9, .L80105018
/* B7C1A8 80105008 97A90068 */   lhu   $t1, 0x68($sp)
/* B7C1AC 8010500C 97AA0066 */  lhu   $t2, 0x66($sp)
/* B7C1B0 80105010 512A000A */  beql  $t1, $t2, .L8010503C
/* B7C1B4 80105014 97AD0060 */   lhu   $t5, 0x60($sp)
.L80105018:
/* B7C1B8 80105018 0C040A38 */  jal   func_801028E0
/* B7C1BC 8010501C 02002025 */   move  $a0, $s0
/* B7C1C0 80105020 50400006 */  beql  $v0, $zero, .L8010503C
/* B7C1C4 80105024 97AD0060 */   lhu   $t5, 0x60($sp)
/* B7C1C8 80105028 8E0B0000 */  lw    $t3, ($s0)
/* B7C1CC 8010502C 356C0004 */  ori   $t4, $t3, 4
/* B7C1D0 80105030 10000040 */  b     .L80105134
/* B7C1D4 80105034 AE0C0000 */   sw    $t4, ($s0)
/* B7C1D8 80105038 97AD0060 */  lhu   $t5, 0x60($sp)
.L8010503C:
/* B7C1DC 8010503C 27A50048 */  addiu $a1, $sp, 0x48
/* B7C1E0 80105040 02002025 */  move  $a0, $s0
/* B7C1E4 80105044 31AE0001 */  andi  $t6, $t5, 1
/* B7C1E8 80105048 55C00016 */  bnezl $t6, .L801050A4
/* B7C1EC 8010504C 8FA40044 */   lw    $a0, 0x44($sp)
/* B7C1F0 80105050 0C040964 */  jal   func_80102590
/* B7C1F4 80105054 27A60020 */   addiu $a2, $sp, 0x20
/* B7C1F8 80105058 10400009 */  beqz  $v0, .L80105080
/* B7C1FC 8010505C 00401825 */   move  $v1, $v0
/* B7C200 80105060 2401000A */  li    $at, 10
/* B7C204 80105064 14410004 */  bne   $v0, $at, .L80105078
/* B7C208 80105068 00000000 */   nop   
/* B7C20C 8010506C 8E0F0000 */  lw    $t7, ($s0)
/* B7C210 80105070 35F80004 */  ori   $t8, $t7, 4
/* B7C214 80105074 AE180000 */  sw    $t8, ($s0)
.L80105078:
/* B7C218 80105078 1000002E */  b     .L80105134
/* B7C21C 8010507C 00601025 */   move  $v0, $v1
.L80105080:
/* B7C220 80105080 97A90038 */  lhu   $t1, 0x38($sp)
/* B7C224 80105084 27B90020 */  addiu $t9, $sp, 0x20
/* B7C228 80105088 AFB90044 */  sw    $t9, 0x44($sp)
/* B7C22C 8010508C 312A0001 */  andi  $t2, $t1, 1
/* B7C230 80105090 55400004 */  bnezl $t2, .L801050A4
/* B7C234 80105094 8FA40044 */   lw    $a0, 0x44($sp)
/* B7C238 80105098 10000026 */  b     .L80105134
/* B7C23C 8010509C 2402000B */   li    $v0, 11
/* B7C240 801050A0 8FA40044 */  lw    $a0, 0x44($sp)
.L801050A4:
/* B7C244 801050A4 2605000C */  addiu $a1, $s0, 0xc
/* B7C248 801050A8 0C001BC4 */  jal   bcopy
/* B7C24C 801050AC 24060020 */   li    $a2, 32
/* B7C250 801050B0 8FAB0044 */  lw    $t3, 0x44($sp)
/* B7C254 801050B4 24190010 */  li    $t9, 16
/* B7C258 801050B8 24090008 */  li    $t1, 8
/* B7C25C 801050BC 916C001B */  lbu   $t4, 0x1b($t3)
/* B7C260 801050C0 8E040004 */  lw    $a0, 4($s0)
/* B7C264 801050C4 8E050008 */  lw    $a1, 8($s0)
/* B7C268 801050C8 AE0C004C */  sw    $t4, 0x4c($s0)
/* B7C26C 801050CC 8FAD0044 */  lw    $t5, 0x44($sp)
/* B7C270 801050D0 24060007 */  li    $a2, 7
/* B7C274 801050D4 2607002C */  addiu $a3, $s0, 0x2c
/* B7C278 801050D8 91AE001A */  lbu   $t6, 0x1a($t5)
/* B7C27C 801050DC AE190050 */  sw    $t9, 0x50($s0)
/* B7C280 801050E0 AE090054 */  sw    $t1, 0x54($s0)
/* B7C284 801050E4 31C200FF */  andi  $v0, $t6, 0xff
/* B7C288 801050E8 000218C0 */  sll   $v1, $v0, 3
/* B7C28C 801050EC 00027840 */  sll   $t7, $v0, 1
/* B7C290 801050F0 24680008 */  addiu $t0, $v1, 8
/* B7C294 801050F4 25F80003 */  addiu $t8, $t7, 3
/* B7C298 801050F8 01035021 */  addu  $t2, $t0, $v1
/* B7C29C 801050FC AE180060 */  sw    $t8, 0x60($s0)
/* B7C2A0 80105100 AE080058 */  sw    $t0, 0x58($s0)
/* B7C2A4 80105104 AE0A005C */  sw    $t2, 0x5c($s0)
/* B7C2A8 80105108 0C0417D0 */  jal   osReadMempak
/* B7C2AC 8010510C A20E0064 */   sb    $t6, 0x64($s0)
/* B7C2B0 80105110 10400003 */  beqz  $v0, .L80105120
/* B7C2B4 80105114 00000000 */   nop   
/* B7C2B8 80105118 10000007 */  b     .L80105138
/* B7C2BC 8010511C 8FBF001C */   lw    $ra, 0x1c($sp)
.L80105120:
/* B7C2C0 80105120 0C041494 */  jal   func_80105250
/* B7C2C4 80105124 02002025 */   move  $a0, $s0
/* B7C2C8 80105128 8E0B0000 */  lw    $t3, ($s0)
/* B7C2CC 8010512C 356C0001 */  ori   $t4, $t3, 1
/* B7C2D0 80105130 AE0C0000 */  sw    $t4, ($s0)
.L80105134:
/* B7C2D4 80105134 8FBF001C */  lw    $ra, 0x1c($sp)
.L80105138:
/* B7C2D8 80105138 8FB00018 */  lw    $s0, 0x18($sp)
/* B7C2DC 8010513C 27BD0070 */  addiu $sp, $sp, 0x70
/* B7C2E0 80105140 03E00008 */  jr    $ra
/* B7C2E4 80105144 00000000 */   nop   

glabel func_80105148
/* B7C2E8 80105148 27BDFF68 */  addiu $sp, $sp, -0x98
/* B7C2EC 8010514C AFBF0024 */  sw    $ra, 0x24($sp)
/* B7C2F0 80105150 AFB00020 */  sw    $s0, 0x20($sp)
/* B7C2F4 80105154 00808025 */  move  $s0, $a0
/* B7C2F8 80105158 0C041320 */  jal   func_80104C80
/* B7C2FC 8010515C 00002825 */   move  $a1, $zero
/* B7C300 80105160 10400003 */  beqz  $v0, .L80105170
/* B7C304 80105164 00003025 */   move  $a2, $zero
/* B7C308 80105168 10000034 */  b     .L8010523C
/* B7C30C 8010516C 8FBF0024 */   lw    $ra, 0x24($sp)
.L80105170:
/* B7C310 80105170 8E040004 */  lw    $a0, 4($s0)
/* B7C314 80105174 8E050008 */  lw    $a1, 8($s0)
/* B7C318 80105178 0C0417D0 */  jal   osReadMempak
/* B7C31C 8010517C 27A70030 */   addiu $a3, $sp, 0x30
/* B7C320 80105180 10400003 */  beqz  $v0, .L80105190
/* B7C324 80105184 27A30070 */   addiu $v1, $sp, 0x70
/* B7C328 80105188 1000002C */  b     .L8010523C
/* B7C32C 8010518C 8FBF0024 */   lw    $ra, 0x24($sp)
.L80105190:
/* B7C330 80105190 00001025 */  move  $v0, $zero
/* B7C334 80105194 24040020 */  li    $a0, 32
.L80105198:
/* B7C338 80105198 244E0001 */  addiu $t6, $v0, 1
/* B7C33C 8010519C 244F0002 */  addiu $t7, $v0, 2
/* B7C340 801051A0 24580003 */  addiu $t8, $v0, 3
/* B7C344 801051A4 A0620000 */  sb    $v0, ($v1)
/* B7C348 801051A8 24420004 */  addiu $v0, $v0, 4
/* B7C34C 801051AC A0780003 */  sb    $t8, 3($v1)
/* B7C350 801051B0 A06F0002 */  sb    $t7, 2($v1)
/* B7C354 801051B4 A06E0001 */  sb    $t6, 1($v1)
/* B7C358 801051B8 1444FFF7 */  bne   $v0, $a0, .L80105198
/* B7C35C 801051BC 24630004 */   addiu $v1, $v1, 4
/* B7C360 801051C0 8E040004 */  lw    $a0, 4($s0)
/* B7C364 801051C4 8E050008 */  lw    $a1, 8($s0)
/* B7C368 801051C8 AFA00010 */  sw    $zero, 0x10($sp)
/* B7C36C 801051CC 00003025 */  move  $a2, $zero
/* B7C370 801051D0 0C04173C */  jal   func_80105CF0
/* B7C374 801051D4 27A70070 */   addiu $a3, $sp, 0x70
/* B7C378 801051D8 10400003 */  beqz  $v0, .L801051E8
/* B7C37C 801051DC 00003025 */   move  $a2, $zero
/* B7C380 801051E0 10000016 */  b     .L8010523C
/* B7C384 801051E4 8FBF0024 */   lw    $ra, 0x24($sp)
.L801051E8:
/* B7C388 801051E8 8E040004 */  lw    $a0, 4($s0)
/* B7C38C 801051EC 8E050008 */  lw    $a1, 8($s0)
/* B7C390 801051F0 0C0417D0 */  jal   osReadMempak
/* B7C394 801051F4 27A70050 */   addiu $a3, $sp, 0x50
/* B7C398 801051F8 10400003 */  beqz  $v0, .L80105208
/* B7C39C 801051FC 27A40070 */   addiu $a0, $sp, 0x70
/* B7C3A0 80105200 1000000E */  b     .L8010523C
/* B7C3A4 80105204 8FBF0024 */   lw    $ra, 0x24($sp)
.L80105208:
/* B7C3A8 80105208 27A50050 */  addiu $a1, $sp, 0x50
/* B7C3AC 8010520C 0C001A30 */  jal   bcmp
/* B7C3B0 80105210 24060020 */   li    $a2, 32
/* B7C3B4 80105214 10400003 */  beqz  $v0, .L80105224
/* B7C3B8 80105218 00003025 */   move  $a2, $zero
/* B7C3BC 8010521C 10000006 */  b     .L80105238
/* B7C3C0 80105220 2402000B */   li    $v0, 11
.L80105224:
/* B7C3C4 80105224 8E040004 */  lw    $a0, 4($s0)
/* B7C3C8 80105228 8E050008 */  lw    $a1, 8($s0)
/* B7C3CC 8010522C AFA00010 */  sw    $zero, 0x10($sp)
/* B7C3D0 80105230 0C04173C */  jal   func_80105CF0
/* B7C3D4 80105234 27A70030 */   addiu $a3, $sp, 0x30
.L80105238:
/* B7C3D8 80105238 8FBF0024 */  lw    $ra, 0x24($sp)
.L8010523C:
/* B7C3DC 8010523C 8FB00020 */  lw    $s0, 0x20($sp)
/* B7C3E0 80105240 27BD0098 */  addiu $sp, $sp, 0x98
/* B7C3E4 80105244 03E00008 */  jr    $ra
/* B7C3E8 80105248 00000000 */   nop   
