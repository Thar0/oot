/**
 * SPDX-FileCopyrightText: Copyright (C) 2024 ZeldaRET
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "aifc.h"
#include "util.h"

#define DEBUGF(fmt, ...) (void)0

typedef struct {
    uint32_t ckID;
    uint32_t ckSize;
} ChunkHeader;

typedef struct {
    ChunkHeader hdr;
    uint32_t formType;
} FormChunk;

typedef struct {
    int16_t numChannels;
    uint16_t numFramesH;
    uint16_t numFramesL;
    int16_t sampleSize;
    int16_t sampleRate[5]; // 80-bit float
    uint16_t compressionTypeH;
    uint16_t compressionTypeL;
    // followed by compression name pstring (?)
} CommonChunk;

typedef struct {
    int16_t MarkerID;
    uint16_t positionH;
    uint16_t positionL;
} Marker;

typedef struct {
    int16_t playMode;
    int16_t beginLoop;
    int16_t endLoop;
} Loop;

typedef struct {
    int8_t baseNote;
    int8_t detune;
    int8_t lowNote;
    int8_t highNote;
    int8_t lowVelocity;
    int8_t highVelocity;
    int16_t gain;
    Loop sustainLoop;
    Loop releaseLoop;
} InstrumentChunk;

typedef struct {
    int32_t offset;
    int32_t blockSize;
} SoundDataChunk;

typedef struct {
    int16_t version;
    int16_t order;
    int16_t nEntries;
} CodeChunk;

#define READ_DATA(file, ptr, length)                  \
    do {                                              \
        if (fread((ptr), (length), 1, (file)) != 1) { \
            error("[aifc:%d] read error", __LINE__);  \
        }                                             \
    } while (0)

static_assert(sizeof(double) == sizeof(uint64_t), "Double is assumed to be 64-bit");

#define F64_GET_SGN(bits)    (((bits) >> 63) & 1)               //  1-bit
#define F64_GET_EXP(bits)    ((((bits) >> 52) & 0x7FF) - 0x3FF) // 15-bit
#define F64_GET_MANT_H(bits) (((bits) >> 32) & 0xFFFFF)         // 20-bit
#define F64_GET_MANT_L(bits) ((bits)&0xFFFFFFFF)                // 32-bit

static UNUSED void
f64_to_f80(double f64, uint8_t *f80)
{
    union {
        uint32_t w[3];
        uint8_t b[12];
    } f80tmp;

    // get f64 bits

    uint64_t f64_bits = *(uint64_t *)&f64;

    int f64_sgn = F64_GET_SGN(f64_bits);
    int f64_exponent = F64_GET_EXP(f64_bits);
    uint32_t f64_mantissa_hi = F64_GET_MANT_H(f64_bits);
    uint32_t f64_mantissa_lo = F64_GET_MANT_L(f64_bits);

    // build f80 words

    f80tmp.w[0] = (f64_sgn << 15) | (f64_exponent + 0x3FFF);
    f80tmp.w[1] = (1 << 31) | (f64_mantissa_hi << 11) | (f64_mantissa_lo >> 21);
    f80tmp.w[2] = f64_mantissa_lo << 11;

    // byteswap to BE

    BSWAP32(f80tmp.w[0]);
    BSWAP32(f80tmp.w[1]);
    BSWAP32(f80tmp.w[2]);

    // write bytes

    for (size_t i = 0; i < 10; i++)
        f80[i] = f80tmp.b[i + 2];
}

static void
f80_to_f64(double *f64, uint8_t *f80)
{
    union {
        uint32_t w[3];
        uint8_t b[12];
    } f80tmp;

    // read bytes

    f80tmp.b[0] = f80tmp.b[1] = 0;
    for (size_t i = 0; i < 10; i++)
        f80tmp.b[i + 2] = f80[i];

    // byteswap from BE

    BSWAP32(f80tmp.w[0]);
    BSWAP32(f80tmp.w[1]);
    BSWAP32(f80tmp.w[2]);

    // get f64 parts

    int f64_sgn = (f80tmp.w[0] >> 15) & 1;
    int f64_exponent = (f80tmp.w[0] & 0x7FFF) - 0x3FFF;
    uint32_t f64_mantissa_hi = (f80tmp.w[1] >> 11) & 0xFFFFF;
    uint32_t f64_mantissa_lo = ((f80tmp.w[1] & 0x7FF) << 21) | (f80tmp.w[2] >> 11);

    // build bitwise f64

    uint64_t f64_bits = ((uint64_t)f64_sgn << 63) | ((((uint64_t)f64_exponent + 0x3FF) & 0x7FF) << 52) |
                        ((uint64_t)f64_mantissa_hi << 32) | ((uint64_t)f64_mantissa_lo);

    // write double

    *f64 = *(double *)&f64_bits;
}

static char *
ReadPString(FILE *f)
{
    unsigned char num;
    char *st;

    // read string length
    READ_DATA(f, &num, sizeof(num));

    // alloc
    st = malloc(num + 1);

    // read string and null terminate it
    READ_DATA(f, st, num);
    st[num] = '\0';

    // pad to 2 byte boundary
    if ((num & 1) == 0)
        READ_DATA(f, &num, sizeof(num));

    return st;
}

#define VADPCM_VER ((int16_t)1)

void
aifc_read(aifc_data *af, const char *path, uint8_t *match_buf, size_t *match_buf_pos)
{
    FILE *aifc_file;
    FormChunk fc;
    bool saw_comm;
    bool saw_ssnd;

    memset(af, 0, sizeof(aifc_data));

    DEBUGF("[aifc] path [%s]\n", path);

    if (path == NULL)
        return;

    aifc_file = fopen(path, "rb");
    if (aifc_file == NULL)
        error("[aifc] sample file [%s] could not be opened", path);

    READ_DATA(aifc_file, &fc, sizeof(fc));

    BSWAP32(fc.hdr.ckID);
    BSWAP32(fc.formType);

    if (fc.hdr.ckID != CC4('F', 'O', 'R', 'M') || fc.formType != CC4('A', 'I', 'F', 'C'))
        error("[aifc] file [%s] is not an AIFF-C file", path);

    af->path = path;

    while (true) {
        ChunkHeader header;
        InstrumentChunk ic;
        CommonChunk cc;
        SoundDataChunk sc;
        uint32_t ts;
        long offset;
        size_t num;

        num = fread(&header, sizeof(header), 1, aifc_file);
        if (num == 0)
            break;

        BSWAP32(header.ckID);
        BSWAP32(header.ckSize);
        header.ckSize++;
        header.ckSize &= ~1;

        offset = ftell(aifc_file);

        DEBUGF("%c%c%c%c\n", header.ckID >> 24, header.ckID >> 16, header.ckID >> 8, header.ckID);

        switch (header.ckID) {
            case CC4('C', 'O', 'M', 'M'):
                READ_DATA(aifc_file, &cc, sizeof(cc));
                BSWAP16(cc.numChannels);
                BSWAP16(cc.numFramesH);
                BSWAP16(cc.numFramesL);
                BSWAP16(cc.sampleSize);
                BSWAP16(cc.compressionTypeH);
                BSWAP16(cc.compressionTypeL);

                assert(cc.numChannels == 1); // mono
                assert(cc.sampleSize == 16); // 16-bit samples

                af->num_channels = cc.numChannels;
                af->sample_size = cc.sampleSize;
                af->num_frames = (cc.numFramesH << 16) | cc.numFramesL;
                af->compression_type = (cc.compressionTypeH << 16) | cc.compressionTypeL;
                f80_to_f64(&af->sample_rate, (uint8_t *)cc.sampleRate);

                af->compression_name = ReadPString(aifc_file);

                DEBUGF("    numChannels %d\n"
                       "    numFrames %u\n"
                       "    sampleSize %d\n"
                       "    sampleRate %f\n"
                       "    compressionType %c%c%c%c (%s)\n",
                       af->num_channels, af->num_frames, af->sample_size, af->sample_rate, af->compression_type >> 24,
                       af->compression_type >> 16, af->compression_type >> 8, af->compression_type,
                       af->compression_name);

                saw_comm = true;
                break;

            case CC4('S', 'S', 'N', 'D'):
                READ_DATA(aifc_file, &sc, sizeof(sc));

                BSWAP32(sc.offset);
                BSWAP32(sc.blockSize);

                assert(sc.offset == 0);
                assert(sc.blockSize == 0);

                af->ssnd_offset = ftell(aifc_file);
                // TODO use numFrames instead?
                af->ssnd_size = header.ckSize - 8;

                // af->data = malloc(af->data_size);
                // READ_DATA(aifc_file, af->data, af->data_size);

                DEBUGF("    offset = 0x%lX size = 0x%lX\n", af->ssnd_offset, af->ssnd_size);

                saw_ssnd = true;
                break;

            case CC4('A', 'P', 'P', 'L'):
                READ_DATA(aifc_file, &ts, sizeof(ts));

                BSWAP32(ts);

                DEBUGF("    %c%c%c%c\n", ts >> 24, ts >> 16, ts >> 8, ts);

                if (ts == CC4('s', 't', 'o', 'c')) {
                    int16_t version;

                    unsigned char name_length;
                    READ_DATA(aifc_file, &name_length, sizeof(name_length));

                    char chunk_name[257];
                    READ_DATA(aifc_file, chunk_name, name_length);
                    chunk_name[name_length] = '\0';

                    DEBUGF("    %s\n", chunk_name);

                    if (strequ(chunk_name, "VADPCMCODES")) {
                        READ_DATA(aifc_file, &version, sizeof(version));
                        BSWAP16(version);

                        if (version != VADPCM_VER)
                            error("Non-identical codebook chunk versions");

                        int16_t order;
                        int16_t npredictors;

                        READ_DATA(aifc_file, &order, sizeof(order));
                        BSWAP16(order);
                        READ_DATA(aifc_file, &npredictors, sizeof(npredictors));
                        BSWAP16(npredictors);

                        uint32_t book_size = 8 * order * npredictors;

                        af->book.order = order;
                        af->book.npredictors = npredictors;
                        af->book_state = malloc(book_size * sizeof(int16_t));
                        READ_DATA(aifc_file, af->book_state, book_size * sizeof(int16_t));
                        for (size_t i = 0; i < book_size; i++)
                            BSWAP16((*af->book_state)[i]);

                        af->has_book = true;

                        // DEBUG

                        DEBUGF("        order       = %d\n"
                               "        npredictors = %d\n",
                               af->book.order, af->book.npredictors);

                        for (size_t i = 0; i < book_size; i++) {
                            if (i % 8 == 0)
                                DEBUGF("\n        ");
                            DEBUGF("%04X ", (uint16_t)(*af->book_state)[i]);
                        }
                        DEBUGF("\n");
                    } else if (strequ(chunk_name, "VADPCMLOOPS")) {
                        READ_DATA(aifc_file, &version, sizeof(version));
                        BSWAP16(version);

                        if (version != VADPCM_VER)
                            error("Non-identical loop chunk versions");

                        int16_t nloops;
                        READ_DATA(aifc_file, &nloops, sizeof(nloops));
                        BSWAP16(nloops);

                        if (nloops != 1)
                            error("Only one loop is supported, got %d", nloops);

                        READ_DATA(aifc_file, &af->loop, sizeof(ALADPCMloop));
                        BSWAP32(af->loop.start);
                        BSWAP32(af->loop.end);
                        BSWAP32(af->loop.count);
                        for (size_t i = 0; i < ARRAY_COUNT(af->loop.state); i++)
                            BSWAP16(af->loop.state[i]);

                        af->has_loop = true;

                        // DEBUG

                        DEBUGF("        start = %d\n"
                               "        end   = %d\n"
                               "        count = %d\n",
                               af->loop.start, af->loop.end, af->loop.count);

                        for (size_t i = 0; i < ARRAY_COUNT(af->loop.state); i++) {
                            if (i % 8 == 0)
                                DEBUGF("\n        ");
                            DEBUGF("%04X ", (uint16_t)af->loop.state[i]);
                        }
                        DEBUGF("\n");
                    }
                }
                break;

            case CC4('I', 'N', 'S', 'T'):
                READ_DATA(aifc_file, &ic, sizeof(ic));

                BSWAP16(ic.gain);
                BSWAP16(ic.sustainLoop.beginLoop);
                BSWAP16(ic.sustainLoop.endLoop);
                BSWAP16(ic.sustainLoop.playMode);
                BSWAP16(ic.releaseLoop.beginLoop);
                BSWAP16(ic.releaseLoop.endLoop);
                BSWAP16(ic.releaseLoop.playMode);

                // basenote

                DEBUGF("    baseNote    = %d (%d)\n"
                       "    detune      = %d\n"
                       "    lowNote     = %d\n"
                       "    highNote    = %d\n"
                       "    lowVelocity = %d\n"
                       "    highVelocity= %d\n"
                       "    gain        = %d\n"
                       "    sustainLoop = %d [%d:%d]\n"
                       "    releaseLoop = %d [%d:%d]\n",
                       ic.baseNote, NOTE_MIDI_TO_Z64(ic.baseNote), ic.detune, ic.lowNote, ic.highNote, ic.lowVelocity,
                       ic.highVelocity, ic.gain, ic.sustainLoop.playMode, ic.sustainLoop.beginLoop,
                       ic.sustainLoop.endLoop, ic.releaseLoop.playMode, ic.releaseLoop.beginLoop,
                       ic.releaseLoop.endLoop);

                af->basenote = ic.baseNote;
                af->detune = ic.detune;
                af->has_inst = true;
                // TODO others?
                break;

            case CC4('M', 'A', 'R', 'K'):;
                int16_t numMarkers;
                READ_DATA(aifc_file, &numMarkers, sizeof(int16_t));
                BSWAP16(numMarkers);

                af->num_markers = numMarkers;
                af->markers = malloc(numMarkers * sizeof(aifc_marker));

                for (int i = 0; i < numMarkers; i++) {
                    Marker marker;

                    READ_DATA(aifc_file, &marker, sizeof(Marker));
                    BSWAP16(marker.MarkerID);
                    BSWAP16(marker.positionH);
                    BSWAP16(marker.positionL);

                    (*af->markers)[i].id = marker.MarkerID;
                    (*af->markers)[i].pos = (marker.positionH << 16) | marker.positionL;
                    (*af->markers)[i].label = ReadPString(aifc_file);

                    DEBUGF("    MARKER: %d @ %u [%s]\n", (*af->markers)[i].id, (*af->markers)[i].pos,
                           (*af->markers)[i].label);
                }
                break;

            default: // skip it
                break;
        }
        fseek(aifc_file, offset + header.ckSize, SEEK_SET);
    }

    if (!saw_comm)
        error("aifc has no COMM chunk");
    if (!saw_ssnd)
        error("aifc file has no data");

    // replicate buffer bug in original tool
    if (match_buf != NULL && match_buf_pos != NULL) {
        size_t buf_pos = ALIGN16(*match_buf_pos) % BUG_BUF_SIZE;
        size_t rem = af->ssnd_size;

        fseek(aifc_file, af->ssnd_offset, SEEK_SET);

        if (rem > BUG_BUF_SIZE - buf_pos) {
            READ_DATA(aifc_file, &match_buf[buf_pos], BUG_BUF_SIZE - buf_pos);
            rem -= BUG_BUF_SIZE - buf_pos;
            buf_pos = 0;
        }
        READ_DATA(aifc_file, &match_buf[buf_pos], rem);

        *match_buf_pos = (buf_pos + rem) % BUG_BUF_SIZE;
    }

    fclose(aifc_file);
}

void
aifc_dispose(aifc_data *af)
{
    free(af->book_state);
    af->has_book = false;

    af->has_loop = false;

    free(af->compression_name);

    for (size_t i = 0; i < af->num_markers; i++)
        free((*af->markers)[i].label);
    free(af->markers);
}
