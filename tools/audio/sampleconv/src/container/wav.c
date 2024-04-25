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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../util.h"
#include "../codec/vadpcm.h"
#include "wav.h"

typedef enum {
    WAVE_TYPE_PCM = 1,
    WAVE_TYPE_FLOAT = 3,
    WAVE_TYPE_ALAW = 6,
    WAVE_TYPE_MULAW = 7,
    WAVE_TYPE_EXTENSIBLE = 0xFFFE,
} wav_type;

typedef enum {
    BIT_DEPTH_8 = 8,
    BIT_DEPTH_16 = 16,
    BIT_DEPTH_24 = 24,
    BIT_DEPTH_32 = 32,
    BIT_DEPTH_64 = 64,
} bit_depth_e;

typedef struct {
    const char *path;
    bit_depth_e bit_depth;
    bool sample_f32;
    unsigned num_samples;
    unsigned num_channels;
    unsigned sample_rate;
    char *sample_data;
    size_t n_sample_data;

    int8_t base_note;
    int8_t fine_tune;
    int8_t gain;
    int8_t key_low;
    int8_t key_hi;
    int8_t vel_low;
    int8_t vel_hi;

    ALADPCMbookhead *book_header;
    ALADPCMbookstate *book_data;

    void *loop;
} container_context;

typedef LITTLE_ENDIAN_STRUCT
{
    uint16_t type;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bit_depth;
}
wav_fmt;

typedef LITTLE_ENDIAN_STRUCT
{
    uint32_t num_samples;
}
wav_fact;

typedef LITTLE_ENDIAN_STRUCT
{
    int8_t base_note;
    int8_t fine_tune;
    int8_t gain;
    int8_t key_low;
    int8_t key_hi;
    int8_t vel_low;
    int8_t vel_hi;
    char _pad[1];
}
wav_inst;

typedef LITTLE_ENDIAN_STRUCT
{
    uint32_t manufacturer;
    uint32_t product;
    uint32_t sample_period;
    uint32_t unity_note;
    char _pad[3];
    uint8_t fine_tune;
    uint32_t format;
    uint32_t offset;
    uint32_t num_sample_loops;
    uint32_t sampler_data;
}
wav_smpl;

typedef LITTLE_ENDIAN_STRUCT
{
    uint32_t cue_point_index;
    uint32_t type;
    uint32_t start;
    uint32_t end;
    uint32_t fraction;
    uint32_t num;
}
wav_loop;

int
wav_read(container_data *out, const char *path, UNUSED bool matching)
{
    bool has_fmt = false;
    bool has_fact = false;
    bool has_data = false;
    bool has_inst = false;
    bool has_smpl = false;
    memset(out, 0, sizeof(*out));

    FILE *in = fopen(path, "rb");
    if (in == NULL)
        error("Failed to open \"%s\" for reading", path);

    char riff[4];
    uint32_t_LE size;
    char wave[4];

    FREAD(in, riff, 4);
    FREAD(in, &size, 4);
    FREAD(in, wave, 4);

    if (!CC4_CHECK(riff, "RIFF") || !CC4_CHECK(wave, "WAVE"))
        error("Not a wav file?");

    while (true) {
        bool skipped = false;

        char cc4[4];
        uint32_t_LE chunk_size;

        long start = ftell(in);
        if (start > 8 + size.value) {
            error("Overran file");
        }
        if (start == 8 + size.value) {
            break;
        }

        FREAD(in, cc4, 4);
        FREAD(in, &chunk_size, 4);

        switch (CC4(cc4[0], cc4[1], cc4[2], cc4[3])) {
            case CC4('f', 'm', 't', ' '): {
                wav_fmt fmt;
                FREAD(in, &fmt, sizeof(fmt));

                // printf("fmt : { %u, %u, %u, %u, %u, %u }\n", fmt.type, fmt.num_channels, fmt.sample_rate,
                // fmt.byte_rate,
                //        fmt.block_align, fmt.bit_depth);

                switch (fmt.type) {
                    case WAVE_TYPE_PCM:
                        out->data_type = SAMPLE_TYPE_PCM16;
                        break;

                    case WAVE_TYPE_FLOAT:
                    case WAVE_TYPE_MULAW:
                    case WAVE_TYPE_ALAW:
                    case WAVE_TYPE_EXTENSIBLE:
                        error("Unhandled sample type: %d", fmt.type);
                        break;

                    default:
                        error("Unrecognized sample type: %d", fmt.type);
                        break;
                }

                out->num_channels = fmt.num_channels;
                out->sample_rate = fmt.sample_rate;
                out->byte_rate = fmt.byte_rate;
                out->block_align = fmt.block_align;
                out->bit_depth = fmt.bit_depth;

                has_fmt = true;
            } break;

            case CC4('f', 'a', 'c', 't'): {
                wav_fact fact;
                FREAD(in, &fact, sizeof(fact));

                // printf("fact: { %u }\n", fact.num_samples);

                out->num_samples = fact.num_samples;

                has_fact = true;
            } break;

            case CC4('d', 'a', 't', 'a'): {
                // printf("data: size=%u\n", chunk_size.value);

                void *data = MALLOC_CHECKED_INFO(chunk_size.value, "data size = %u", chunk_size.value);
                FREAD(in, data, chunk_size.value);

                out->data = data;
                out->data_size = chunk_size.value;

                has_data = true;
            } break;

            case CC4('i', 'n', 's', 't'): {
                wav_inst inst;
                FREAD(in, &inst, sizeof(inst));

                // printf("inst: { %u, %u, %u, %u, %u, %u, %u }\n", inst.base_note, inst.fine_tune, inst.gain,
                //        inst.key_low, inst.key_hi, inst.vel_low, inst.vel_hi);

                out->base_note = inst.base_note;
                out->fine_tune = inst.fine_tune;
                out->gain = inst.gain;
                out->key_low = inst.key_low;
                out->key_hi = inst.key_hi;
                out->vel_low = inst.vel_low;
                out->vel_hi = inst.vel_hi;

                has_inst = true;
            } break;

            case CC4('s', 'm', 'p', 'l'): {
                wav_smpl smpl;
                FREAD(in, &smpl, sizeof(smpl));

                // printf("smpl: { %u, %u, %u, %u, %u, %u, %u, %u, %u }\n", smpl.manufacturer, smpl.product,
                //        smpl.sample_period, smpl.unity_note, smpl.fine_tune, smpl.format, smpl.offset,
                //        smpl.num_sample_loops, smpl.sampler_data);

                // seems useless
                // out->smpl.manufacturer = smpl.manufacturer;
                // out->smpl.product = smpl.product;
                // out->smpl.sample_period = smpl.sample_period;
                // out->smpl.format = smpl.format;
                // out->smpl.offset = smpl.offset;
                // out->smpl.sampler_data = smpl.sampler_data;

                // seems maybe important
                // out->smpl.unity_note = smpl.unity_note;
                // out->smpl.fine_tune = smpl.fine_tune;

                out->num_loops = smpl.num_sample_loops;
                if (out->num_loops != 0) {
                    out->loops =
                        MALLOC_CHECKED_INFO(out->num_loops * sizeof(container_loop), "num_loops=%u", out->num_loops);

                    for (size_t i = 0; i < out->num_loops; i++) {
                        wav_loop loop;
                        FREAD(in, &loop, sizeof(loop));

                        // printf("    loop: { %u, %u, %u, %u, %u, %u }\n", loop.cue_point_index, loop.type, loop.start,
                        //        loop.end, loop.fraction, loop.num);

                        // out->loops[i].cue_point_index = loop.cue_point_index;

                        loop_type type;
                        switch (loop.type) {
                            case 0:
                                type = LOOP_FORWARD;
                                break;

                            case 1:
                                type = LOOP_FORWARD_BACKWARD;
                                break;

                            case 2:
                                type = LOOP_BACKWARD;
                                break;

                            default:
                                error("Unrecognized loop type in wav");
                        }

                        out->loops[i].id = i;
                        out->loops[i].type = type;
                        out->loops[i].start = loop.start;
                        out->loops[i].end = loop.end;
                        out->loops[i].fraction = loop.fraction;
                        out->loops[i].num = loop.num;
                    }
                }
                has_smpl = true;
            } break;

            case CC4('z', 'z', 'b', 'k'): {
                char vadpcmcodes[12];
                uint16_t_LE version;
                uint16_t_LE order;
                uint16_t_LE npredictors;

                FREAD(in, vadpcmcodes, sizeof(vadpcmcodes));
                FREAD(in, &version, sizeof(version));
                FREAD(in, &order, sizeof(order));
                FREAD(in, &npredictors, sizeof(npredictors));

                // printf("VADPCM book: order=%u npredictors=%u\n", order.value, npredictors.value);

                size_t book_size = VADPCM_BOOK_SIZE(order.value, npredictors.value);
                size_t book_data_size = sizeof(int16_t) * book_size;
                int16_t *book_data =
                    MALLOC_CHECKED_INFO(book_data_size, "order=%u, npredictors=%u", order.value, npredictors.value);
                FREAD(in, book_data, book_data_size);

                out->vadpcm.book_header.order = order.value;
                out->vadpcm.book_header.npredictors = npredictors.value;
                out->vadpcm.book_data = book_data;

                for (size_t i = 0; i < book_size; i++) {
                    out->vadpcm.book_data[i] = le16toh(out->vadpcm.book_data[i]);
                }

                out->vadpcm.has_book = true;
            } break;

            case CC4('z', 'z', 'l', 'p'): {
                uint16_t_LE version;
                uint16_t_LE nloops;

                FREAD(in, &version, sizeof(version));
                FREAD(in, &nloops, sizeof(nloops));

                if (nloops.value != 0)
                    out->vadpcm.loops =
                        MALLOC_CHECKED_INFO(nloops.value * sizeof(ALADPCMloop), "nloops=%u", nloops.value);

                for (size_t i = 0; i < nloops.value; i++) {
                    uint32_t_LE loop_start;
                    uint32_t_LE loop_end;
                    uint32_t_LE loop_num;

                    FREAD(in, &loop_start, sizeof(loop_start));
                    FREAD(in, &loop_end, sizeof(loop_end));
                    FREAD(in, &loop_num, sizeof(loop_num));

                    out->vadpcm.loops[i].start = loop_start.value;
                    out->vadpcm.loops[i].end = loop_end.value;
                    out->vadpcm.loops[i].count = loop_num.value;

                    // printf("VADPCM loop (start=%u, end=%u, num=%u)\n", loop_start.value, loop_end.value,
                    //        loop_num.value);

                    if (out->vadpcm.loops[i].count != 0) {
                        FREAD(in, out->vadpcm.loops[i].state, sizeof(out->vadpcm.loops[i].state));

                        for (size_t j = 0; j < ARRAY_COUNT(out->vadpcm.loops[i].state); j++) {
                            out->vadpcm.loops[i].state[j] = le16toh(out->vadpcm.loops[i].state[j]);
                        }

                        // printf("    LOOP STATE = { %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d,
                        // %d }\n",
                        //        out->vadpcm.loops[i].state[0], out->vadpcm.loops[i].state[1],
                        //        out->vadpcm.loops[i].state[2], out->vadpcm.loops[i].state[3],
                        //        out->vadpcm.loops[i].state[4], out->vadpcm.loops[i].state[5],
                        //        out->vadpcm.loops[i].state[6], out->vadpcm.loops[i].state[7],
                        //        out->vadpcm.loops[i].state[8], out->vadpcm.loops[i].state[9],
                        //        out->vadpcm.loops[i].state[10], out->vadpcm.loops[i].state[11],
                        //        out->vadpcm.loops[i].state[12], out->vadpcm.loops[i].state[13],
                        //        out->vadpcm.loops[i].state[14], out->vadpcm.loops[i].state[15]);
                    }
                }
            } break;

#if 0
            case CC4('z', 'z', 'f', 'm'): {
                char fmt[4];
                FREAD(in, fmt, 4);

                printf("zzfm=\"%.*s\"\n", 4,fmt);
            } break;
#endif

            default:
                warning("Skipping unknown wav chunk: \"%c%c%c%c\"", cc4[0], cc4[1], cc4[2], cc4[3]);
                skipped = true;
                break;
        }

        long read_size = ftell(in) - start - 8;

        if (read_size > chunk_size.value)
            error("overran chunk");
        else if (!skipped && read_size < chunk_size.value)
            warning("did not read entire %*s chunk: %lu vs %u", 4, cc4, read_size, chunk_size.value);

        fseek(in, start + 8 + chunk_size.value, SEEK_SET);
    }

    if (!has_fmt)
        error("wav has no fmt chunk");
    if (!has_data)
        error("wav has no data chunk");

    if (!has_fact) {
        assert(out->data_type == SAMPLE_TYPE_PCM16);
        out->num_samples = out->data_size / (out->bit_depth / 8);
    }

    if (!has_inst) {
        out->base_note = 60; // C4
        out->fine_tune = 0;
        out->gain = 0;
        out->key_low = 0;
        out->key_hi = 0;
        out->vel_low = 0;
        out->vel_hi = 0;
    }

    if (!has_smpl) {
        out->num_loops = 0;
        out->loops = NULL;
    }

    if (out->data_type == SAMPLE_TYPE_PCM16) {
        assert(out->data_size % 2 == 0);
        assert(out->bit_depth == 16);

        for (size_t i = 0; i < out->data_size / 2; i++)
            ((uint16_t *)(out->data))[i] = le16toh(((uint16_t *)(out->data))[i]);
    }

    fclose(in);
    return 0;
}

int
wav_write(container_data *in, const char *path, bool matching)
{
    long chunk_start;

    FILE *out = fopen(path, "wb");
    FWRITE(out, "RIFF\0\0\0\0WAVE", 12);

    // frame_size = bit_depth / 8;

    uint16_t fmt_type;
    switch (in->data_type) {
        case SAMPLE_TYPE_PCM16:
            fmt_type = WAVE_TYPE_PCM;
            break;

        default:
            error("Unrecognized sample type for wav output");
            break;
    }

    wav_fmt fmt = {
        .type = fmt_type,
        .num_channels = in->num_channels,
        .sample_rate = in->sample_rate,
        .byte_rate = in->sample_rate * (in->bit_depth / 8),
        .block_align = in->num_channels * (in->bit_depth / 8),
        .bit_depth = in->bit_depth,
    };
    CHUNK_BEGIN(out, "fmt ", &chunk_start);
    CHUNK_WRITE(out, &fmt);
    CHUNK_END(out, chunk_start, uint32_t_LE);

    wav_fact fact = {
        .num_samples = in->num_samples,
    };
    CHUNK_BEGIN(out, "fact", &chunk_start);
    CHUNK_WRITE(out, &fact);
    CHUNK_END(out, chunk_start, uint32_t_LE);

    if (in->data_type == SAMPLE_TYPE_PCM16) {
        assert(in->data_size % 2 == 0);
        assert(in->bit_depth == 16);

        for (size_t i = 0; i < in->data_size / 2; i++) {
            ((uint16_t *)in->data)[i] = htole16(((uint16_t *)in->data)[i]);
        }
    }

    CHUNK_BEGIN(out, "data", &chunk_start);
    CHUNK_WRITE_RAW(out, in->data, in->data_size);
    CHUNK_END(out, chunk_start, uint32_t_LE);

    wav_inst inst = {
        .base_note = in->base_note,
        .fine_tune = in->fine_tune,
        .gain = in->gain,
        .key_low = in->key_low,
        .key_hi = in->key_hi,
        .vel_low = in->vel_low,
        .vel_hi = in->vel_hi,
        ._pad = { 0 },
    };
    CHUNK_BEGIN(out, "inst", &chunk_start);
    CHUNK_WRITE(out, &inst);
    CHUNK_END(out, chunk_start, uint32_t_LE);

    wav_smpl smpl = {
        .manufacturer = 0,
        .product = 0,
        .sample_period = 0,
        .unity_note = 0,
        ._pad = {0, 0, 0},
        .fine_tune = 0,
        .format = 0,
        .offset = 0,
        .num_sample_loops = in->num_loops,
        .sampler_data = 0,
    };
    CHUNK_BEGIN(out, "smpl", &chunk_start);
    CHUNK_WRITE(out, &smpl);
    for (size_t i = 0; i < in->num_loops; i++) {
        uint32_t wav_loop_type;
        switch (in->loops[i].type) {
            case LOOP_FORWARD:
                wav_loop_type = 0;
                break;

            case LOOP_FORWARD_BACKWARD:
                wav_loop_type = 1;
                break;

            case LOOP_BACKWARD:
                wav_loop_type = 2;
                break;

            default:
                error("Unrecognized loop type for wav output");
        }

        wav_loop loop = {
            .cue_point_index = 0,
            .type = wav_loop_type,
            .start = in->loops[i].start,
            .end = in->loops[i].end,
            .fraction = in->loops[i].fraction,
            .num = in->loops[i].num,
        };
        CHUNK_WRITE(out, &loop);
    }
    CHUNK_END(out, chunk_start, uint32_t_LE);

    if (in->vadpcm.has_book && matching) {
        uint32_t book_size = VADPCM_BOOK_SIZE(in->vadpcm.book_header.order, in->vadpcm.book_header.npredictors);

        uint16_t_LE version = { in->vadpcm.book_version };
        uint16_t_LE order = { in->vadpcm.book_header.order };
        uint16_t_LE npredictors = { in->vadpcm.book_header.npredictors };

        int16_t book_state[book_size];
        for (size_t i = 0; i < book_size; i++) {
            book_state[i] = htole16(in->vadpcm.book_data[i]);
        }

        CHUNK_BEGIN(out, "zzbk", &chunk_start);
        CHUNK_WRITE_RAW(out, "VADPCMCODES", 12);
        CHUNK_WRITE(out, &version);
        CHUNK_WRITE(out, &order);
        CHUNK_WRITE(out, &npredictors);
        CHUNK_WRITE_RAW(out, book_state, sizeof(int16_t) * book_size);
        CHUNK_END(out, chunk_start, uint32_t_LE);
    }

    if (in->vadpcm.num_loops != 0) {
        CHUNK_BEGIN(out, "zzlp", &chunk_start);

        for (size_t i = 0; i < in->vadpcm.num_loops; i++) {
            uint32_t_LE loop_start = { in->vadpcm.loops[i].start };
            uint32_t_LE loop_end = { in->vadpcm.loops[i].end };
            uint32_t_LE loop_count = { in->vadpcm.loops[i].count };

            int16_t loop_state[16];
            for (size_t j = 0; j < ARRAY_COUNT(in->vadpcm.loops[i].state); j++) {
                loop_state[j] = htole16(in->vadpcm.loops[i].state[j]);
            }

            CHUNK_WRITE(out, &loop_start);
            CHUNK_WRITE(out, &loop_end);
            CHUNK_WRITE(out, &loop_count);
            CHUNK_WRITE_RAW(out, loop_state, sizeof(loop_state));
        }

        CHUNK_END(out, chunk_start, uint32_t_LE);
    }

#if 0
    CHUNK_BEGIN(out, "zzfm", &chunk_start);
    CHUNK_WRITE_RAW(out, in->zzfm.fmt, 4);
    CHUNK_END(out, chunk_start, uint32_t_LE);
#endif

    uint32_t_LE size = { ftell(out) - 8 };
    fseek(out, 4, SEEK_SET);
    fwrite(&size, 4, 1, out);
    fclose(out);

    return 0;
}
