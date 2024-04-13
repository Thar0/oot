#ifndef AIFF_H
#define AIFF_H

#include <stddef.h>
#include <stdint.h>

#include "../codec/vadpcm.h"
#include "container.h"

#if 0
typedef struct __attribute__((deprecated)) {
    // COMM
    struct {
        int16_t numChannels;
        uint32_t numFrames;
        int16_t sampleSize;
        double sampleRate;
        uint32_t compressionType;
        char *compression_name;
    } comm;
    bool has_comm;
    // INST
    struct {
        int8_t baseNote;
        int8_t detune;
        int8_t lowNote;
        int8_t highNote;
        int8_t lowVelocity;
        int8_t highVelocity;
        int16_t gain;
        struct {
            int16_t playMode;
            int16_t beginLoop;
            int16_t endLoop;
        } sustainLoop;
        struct {
            int16_t playMode;
            int16_t beginLoop;
            int16_t endLoop;
        } releaseLoop;
    } inst;
    bool has_inst;
    // MARK
    struct {
        char _tmp;
    } mark;
    bool has_mark;
    // APPL::stoc::VADPCMCODES
    struct {
        int16_t version;
        int16_t order;
        int16_t npredictors;
        int16_t *book;
    } vadpcmcodes;
    bool has_vadpcmcodes;
    // APPL::stoc::VADPCMLOOPS
    struct {
        int16_t version;
        int16_t nloops;
        ALADPCMloop loop;
    } vadpcmloops;
    bool has_vadpcmloops;
    // SSND
    struct {
        int32_t offset;
        int32_t blockSize;
        void *data;
        size_t data_size;
    } ssnd;
    bool has_ssnd;
} aiff_data;
#endif

int
aifc_read(container_data *out, const char *path, bool matching);
int
aiff_read(container_data *out, const char *path, bool matching);

int
aifc_write(container_data *in, const char *path, bool matching);
int
aiff_write(container_data *in, const char *path, bool matching);

#endif
