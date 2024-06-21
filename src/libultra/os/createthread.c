#include "global.h"
#include "ultra64/asm.h"

void osCreateThread(OSThread* thread, OSId id, void (*entry)(void*), void* arg, void* sp, OSPri pri) {
    register u32 prevInt;
    OSIntMask mask;

    thread->id = id;
    thread->priority = pri;
    thread->next = NULL;
    thread->queue = NULL;
    thread->context.pc = (u32)entry;
    thread->context.a0 = (u64)(s32)arg;
    thread->context.sp = (u64)(s32)sp - FRAMESZ(SZREG * NARGSAVE);
    thread->context.ra = (u64)(s32)__osCleanupThread;

    mask = OS_IM_ALL;
#if !defined(_MIPS_SIM) || _MIPS_SIM == _ABIO32
    // EABI or O32
    thread->context.sr = (mask & OS_IM_CPU) | SR_EXL;
#else
    // O64 or NABI
    thread->context.sr = (mask & OS_IM_CPU) | SR_EXL | SR_FR;
#endif
    thread->context.rcp = (mask & RCP_IMASK) >> RCP_IMASKSHIFT;
    thread->context.fpcsr = FPCSR_FS | FPCSR_EV;
    thread->fp = 0;
    thread->state = OS_STATE_STOPPED;
    thread->flags = 0;

    prevInt = __osDisableInt();
    thread->tlnext = __osActiveQueue;
    __osActiveQueue = thread;
    __osRestoreInt(prevInt);
}
