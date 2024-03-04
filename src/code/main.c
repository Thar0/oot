#include "global.h"
#include "terminal.h"

extern u8 _buffersSegmentEnd[];

s32 gScreenWidth = SCREEN_WIDTH;
s32 gScreenHeight = SCREEN_HEIGHT;
u32 gSystemHeapSize = 0;

PreNmiBuff* gAppNmiBufferPtr;
Scheduler gScheduler;
PadMgr gPadMgr;
IrqMgr gIrqMgr;
uintptr_t gSegments[NUM_SEGMENTS];
OSThread sGraphThread;
STACK(sGraphStack, 0x1800);
STACK(sSchedStack, 0x600);
STACK(sAudioStack, 0x800);
STACK(sPadMgrStack, 0x500);
STACK(sIrqMgrStack, 0x500);
StackEntry sGraphStackInfo;
StackEntry sSchedStackInfo;
StackEntry sAudioStackInfo;
StackEntry sPadMgrStackInfo;
StackEntry sIrqMgrStackInfo;
AudioMgr gAudioMgr;
OSMesgQueue sSerialEventQueue;
OSMesg sSerialMsgBuf[1];

#if OOT_DEBUG
void Main_LogSystemHeap(void) {
    PRINTF(VT_FGCOL(GREEN));
    // "System heap size% 08x (% dKB) Start address% 08x"
    PRINTF("システムヒープサイズ %08x(%dKB) 開始アドレス %08x\n", gSystemHeapSize, gSystemHeapSize / 1024,
           _buffersSegmentEnd);
    PRINTF(VT_RST);
}
#endif

void Main(void* arg) {
    IrqMgrClient irqClient;
    OSMesgQueue irqMgrMsgQueue;
    OSMesg irqMgrMsgBuf[60];
    uintptr_t systemHeapStart;
    uintptr_t fb;

    PRINTF("mainproc 実行開始\n"); // "Start running"
    gScreenWidth = SCREEN_WIDTH;
    gScreenHeight = SCREEN_HEIGHT;
    gAppNmiBufferPtr = (PreNmiBuff*)osAppNMIBuffer;
    PreNmiBuff_Init(gAppNmiBufferPtr);
    Fault_Init();
    SysCfb_Init(0);
    systemHeapStart = (uintptr_t)_buffersSegmentEnd;
    fb = (uintptr_t)SysCfb_GetFbPtr(0);
    gSystemHeapSize = fb - systemHeapStart;
    // "System heap initalization"
    PRINTF("システムヒープ初期化 %08x-%08x %08x\n", systemHeapStart, fb, gSystemHeapSize);
    SystemHeap_Init((void*)systemHeapStart, gSystemHeapSize); // initializes the system heap

#if OOT_DEBUG
    {
        void* debugHeapStart;
        u32 debugHeapSize;

        if (osMemSize >= 0x800000) {
            debugHeapStart = SysCfb_GetFbEnd();
            debugHeapSize = PHYS_TO_K0(0x800000) - (uintptr_t)debugHeapStart;
        } else {
            debugHeapSize = 0x400;
            debugHeapStart = SYSTEM_ARENA_MALLOC(debugHeapSize, "../main.c", 565);
        }

        PRINTF("debug_InitArena(%08x, %08x)\n", debugHeapStart, debugHeapSize);
        DebugArena_Init(debugHeapStart, debugHeapSize);
    }
#endif

    Regs_Init();

    R_ENABLE_ARENA_DBG = 0;

    osCreateMesgQueue(&sSerialEventQueue, sSerialMsgBuf, ARRAY_COUNT(sSerialMsgBuf));
    osSetEventMesg(OS_EVENT_SI, &sSerialEventQueue, NULL);

#if OOT_DEBUG
    Main_LogSystemHeap();
#endif

    osCreateMesgQueue(&irqMgrMsgQueue, irqMgrMsgBuf, ARRAY_COUNT(irqMgrMsgBuf));
    StackCheck_Init(&sIrqMgrStackInfo, sIrqMgrStack, STACK_TOP(sIrqMgrStack), 0, 0x100, "irqmgr");
    IrqMgr_Init(&gIrqMgr, STACK_TOP(sIrqMgrStack), THREAD_PRI_IRQMGR, 1);

    PRINTF("タスクスケジューラの初期化\n"); // "Initialize the task scheduler"
    StackCheck_Init(&sSchedStackInfo, sSchedStack, STACK_TOP(sSchedStack), 0, 0x100, "sched");
    Sched_Init(&gScheduler, STACK_TOP(sSchedStack), THREAD_PRI_SCHED, gViConfigModeType, 1, &gIrqMgr);

    IrqMgr_AddClient(&gIrqMgr, &irqClient, &irqMgrMsgQueue);

    StackCheck_Init(&sAudioStackInfo, sAudioStack, STACK_TOP(sAudioStack), 0, 0x100, "audio");
    AudioMgr_Init(&gAudioMgr, STACK_TOP(sAudioStack), THREAD_PRI_AUDIOMGR, THREAD_ID_AUDIOMGR, &gScheduler, &gIrqMgr);

    StackCheck_Init(&sPadMgrStackInfo, sPadMgrStack, STACK_TOP(sPadMgrStack), 0, 0x100, "padmgr");
    PadMgr_Init(&gPadMgr, &sSerialEventQueue, &gIrqMgr, THREAD_ID_PADMGR, THREAD_PRI_PADMGR, STACK_TOP(sPadMgrStack));

    AudioMgr_WaitForInit(&gAudioMgr);

    StackCheck_Init(&sGraphStackInfo, sGraphStack, STACK_TOP(sGraphStack), 0, 0x100, "graph");
    osCreateThread(&sGraphThread, THREAD_ID_GRAPH, Graph_ThreadEntry, arg, STACK_TOP(sGraphStack), THREAD_PRI_GRAPH);
    osStartThread(&sGraphThread);
    osSetThreadPri(NULL, THREAD_PRI_MAIN);

    while (true) {
        s16* msg = NULL;

        osRecvMesg(&irqMgrMsgQueue, (OSMesg*)&msg, OS_MESG_BLOCK);
        if (msg == NULL) {
            break;
        }
        if (*msg == OS_SC_PRE_NMI_MSG) {
            PRINTF("main.c: リセットされたみたいだよ\n"); // "Looks like it's been reset"
            PreNmiBuff_SetReset(gAppNmiBufferPtr);
        }
    }

    PRINTF("mainproc 後始末\n"); // "Cleanup"
    osDestroyThread(&sGraphThread);
    RcpUtils_Reset();
    PRINTF("mainproc 実行終了\n"); // "End of execution"
}
