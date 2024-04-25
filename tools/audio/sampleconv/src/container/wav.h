/**
 * SPDX-FileCopyrightText: Copyright (C) 2024 ZeldaRET
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef WAV_H
#define WAV_H

#include "../codec/vadpcm.h"
#include "container.h"

#if 0
typedef struct __attribute__((deprecated)) {
    // fmt
    struct {
        uint16_t type;
        uint16_t num_channels;
        uint32_t sample_rate;
        uint32_t byte_rate;
        uint16_t block_align;
        uint16_t bit_depth;
    } fmt;
    bool has_fmt;
    // fact
    struct {
        uint32_t num_samples;
    } fact;
    bool has_fact;
    // inst
    struct {
        int8_t base_note;
        int8_t fine_tune;
        int8_t gain;
        int8_t key_low;
        int8_t key_hi;
        int8_t vel_low;
        int8_t vel_hi;
    } inst;
    bool has_inst;
    // smpl
    struct {
        uint32_t manufacturer;
        uint32_t product;
        uint32_t sample_period;
        uint32_t unity_note;
        uint8_t fine_tune;
        uint32_t format;
        uint32_t offset;
        uint32_t num_sample_loops;
        uint32_t sampler_data;
        struct {
            uint32_t cue_point_index;
            uint32_t type;
            uint32_t start;
            uint32_t end;
            uint32_t fraction;
            uint32_t num;
        } loop;
    } smpl;
    bool has_smpl;
    // data
    struct {
        void *data;
        size_t data_size;
    } data;
    bool has_data;
    // zzbk
    struct {
        int16_t version;
        int16_t order;
        int16_t npredictors;
        int16_t *book;
    } zzbk;
    bool has_zzbk;
    // zzlp
    struct {
        int16_t version;
        int16_t nloops;
        ALADPCMloop loop;
    } zzlp;
    bool has_zzlp;
    // zzfm
    struct {
        char fmt[4];
    } zzfm;
    bool has_zzfm;
} wav_data;
#endif

int
wav_write(container_data *in, const char *path, bool matching);
int
wav_read(container_data *out, const char *path, bool matching);

#endif
