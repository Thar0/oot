#ifndef SOUNDFONT_FILE_H
#define SOUNDFONT_FILE_H

#include "libc/stdbool.h"
#include "../alignment.h"
#include "../attributes.h"
#include "../z64audio.h"

#define DATA __attribute__((section(".data")))

#ifndef GLUE
#define GLUE(a,b) a##b
#endif
#ifndef GLUE2
#define GLUE2(a,b) GLUE(a,b)
#endif

#define ENVELOPE_POINT(delay, target) { (delay),      (target) }
#define ENVELOPE_DISABLE()            { ADSR_DISABLE, 0        }
#define ENVELOPE_HANG()               { ADSR_HANG,    0        }
#define ENVELOPE_GOTO(index)          { ADSR_GOTO,    (index)  }
#define ENVELOPE_RESTART()            { ADSR_RESTART, 0        }

#define INSTR_SAMPLE_NONE { NULL, 0.0f }
#define INSTR_SAMPLE_LO_NONE 0
#define INSTR_SAMPLE_HI_NONE 127

#ifdef __sgi
#define SF_PAD4() static u8 GLUE2(_pad, __LINE__) [] = { 0,0,0,0 }
#define SF_PAD8() static u8 GLUE2(_pad, __LINE__) [] = { 0,0,0,0,0,0,0,0 }
#define SF_PADC() static u8 GLUE2(_pad, __LINE__) [] = { 0,0,0,0,0,0,0,0,0,0,0,0 }
#else
#define SF_PAD4()
#define SF_PAD8()
#define SF_PADC()
#endif

#endif
