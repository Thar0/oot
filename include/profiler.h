#ifndef PROFILER_H
#define PROFILER_H

#include "ultra64.h"
#include "libu64/gfxprint.h"

struct GraphicsContext;

#define PROF_RINGBUFFER_LEN 20

typedef struct {
    OSTime buffer[PROF_RINGBUFFER_LEN];
    u32 bufferIndex;
    OSTime start;
    OSTime time;
} Profiler;

extern u32 gProfilerEnabled;

extern Profiler gPlayUpdateProfiler;
extern Profiler gCollisionCheckProfiler;
extern Profiler gActorUpdateProfiler;

void Profiler_UpdateAndDraw(struct GraphicsContext* gfxCtx);

/* #ifndef NDEBUG */
#if 1
static inline __attribute__((always_inline))
void Profiler_Start(Profiler* prof) {
    prof->start = osGetTime();
}

static inline __attribute__((always_inline))
void Profiler_End(Profiler* prof) {
    prof->time = osGetTime() - prof->start;
}
#else
#define Profiler_Start(prof)
#define Profiler_End(prof)
#endif

#endif
