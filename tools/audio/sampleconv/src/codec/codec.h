#ifndef CODEC_H_
#define CODEC_H_

#include <stdint.h>

#include "../container/container.h"

#include "vadpcm.h"
#include "uncompressed.h"

typedef struct enc_dec_opts {
    // Matching
    bool matching;

    // VADPCM options
    bool truncate;
    uint32_t min_loop_length;
    table_design_spec design;
} enc_dec_opts;

typedef struct codec_spec {
    const char *name;
    sample_data_type type;
    int frame_size;
    bool compressed;
    int (*decode)(container_data *ctnr, const struct codec_spec *codec, const struct enc_dec_opts *opts);
    int (*encode)(container_data *ctnr, const struct codec_spec *codec, const struct enc_dec_opts *opts);
} codec_spec;

#endif
