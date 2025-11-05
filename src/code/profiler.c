#include "profiler.h"
#include "gfx.h"
#include "gfxalloc.h"
#include "speed_meter.h"
#include "array_count.h"

u32 gProfilerEnabled = true;

#if 0 /* PLATFORM_IQUE */
#define CPU_COUNT (144000000/2)
#define RCP_COUNT  96000000
#else
#define CPU_COUNT  (93750000/2)
#define RCP_COUNT  62500000
#endif

static void Profiler_RingBufferUpdate(Profiler* profiler) {
    u32 cur = profiler->bufferIndex;

    profiler->buffer[cur] = profiler->time;

    u32 next = cur + 1;
    if (next >= PROF_RINGBUFFER_LEN) {
        next = 0;
    }
    profiler->bufferIndex = next;
}

static f32 Profiler_CalcFPS(Profiler* profiler) {
    f32 sum = 0.0f;
    for (u32 i = 0; i < PROF_RINGBUFFER_LEN; i++) {
        sum += (f32)(u32)profiler->buffer[i];
    }
    // <OSTime[PROF_RINGBUFFER_LEN]> to 1/sec
    return (CPU_COUNT * PROF_RINGBUFFER_LEN) / sum;
}

static f32 Profiler_CalcUsec(Profiler* profiler) {
    f32 sum = 0.0f;
    for (u32 i = 0; i < PROF_RINGBUFFER_LEN; i++) {
        sum += (f32)(u32)profiler->buffer[i];
    }
    // <OSTime[PROF_RINGBUFFER_LEN]> to usec
    return (1000000 * sum) / (CPU_COUNT * PROF_RINGBUFFER_LEN);
}

#define RCP_CYCLES_TO_USEC(c)   (((u64)(c) * (1000000LL / 15625LL)) / (RCP_COUNT / 15625LL))

Profiler gPlayUpdateProfiler;

Profiler gCollisionCheckProfiler;
Profiler gActorUpdateProfiler;
Profiler gCameraUpdateProfiler;

void Profiler_UpdateAndDraw(GraphicsContext* gfxCtx) {
    if (!gProfilerEnabled) {
        return;
    }

    static Profiler fpsProfiler;
    static Profiler cpuGraphThreadTime;

    static Profiler* sMiscProfilers[] = {
        // FPS
        &fpsProfiler,
        &cpuGraphThreadTime,
    };

    static struct {
        Profiler* prof;
        const char* name;
    } sCpuProfilers[] = {
        { &gPlayUpdateProfiler, "Play_Update"},
        // { &gCollisionCheckProfiler, "Collision Check"},
        { &gActorUpdateProfiler, "Actor_UpdateAll"},
        // { &gCameraUpdateProfiler, "Camera Update"},
    };

    // FPS times
    Profiler_End(&fpsProfiler);
    Profiler_Start(&fpsProfiler);

    // Thread times
    cpuGraphThreadTime.time = gGfxTaskSentToNextReadyMinusAudioThreadUpdateTime;

    // Update ringbuffers
    for (u32 i = 0; i < ARRAY_COUNT(sMiscProfilers); i++) {
        Profiler_RingBufferUpdate(sMiscProfilers[i]);
    }
    for (u32 i = 0; i < ARRAY_COUNT(sCpuProfilers); i++) {
        Profiler_RingBufferUpdate(sCpuProfilers[i].prof);
    }

    // Present

    GfxPrint gfxP;
    Gfx* polyOpaP;
    Gfx* dl;

    OPEN_DISPS(gfxCtx, __FILE__, __LINE__);

    dl = Gfx_Open(polyOpaP = POLY_OPA_DISP);
    gSPDisplayList(OVERLAY_DISP++, dl);

    GfxPrint_Init(&gfxP);
    GfxPrint_SetBasePosPx(&gfxP, 1 * GFX_CHAR_X_SPACING, 1 * GFX_CHAR_Y_SPACING);
    GfxPrint_SetPos(&gfxP, 0, 0);
    GfxPrint_Open(&gfxP, dl);
    GfxPrint_SetColor(&gfxP, 255, 255, 0, 255);
    gfxP.flags &= ~GFXP_FLAG_SHADOW; // halves the number of gbi commands

    GfxPrint_Printf(&gfxP, "FPS: %.2f\n"
                           "CPU:\n"
                           "    GRAPH = %.2fus\n",
                    Profiler_CalcFPS(&fpsProfiler),
                    Profiler_CalcUsec(&cpuGraphThreadTime));

    for (u32 i = 0; i < ARRAY_COUNT(sCpuProfilers); i++) {
        GfxPrint_Printf(&gfxP, "    %s = %.2fus\n", sCpuProfilers[i].name, Profiler_CalcUsec(sCpuProfilers[i].prof));
    }

    dl = GfxPrint_Close(&gfxP);
    GfxPrint_Destroy(&gfxP);

    gSPEndDisplayList(dl++);
    Gfx_Close(polyOpaP, dl);
    POLY_OPA_DISP = dl;

    CLOSE_DISPS(gfxCtx, __FILE__, __LINE__);
}
