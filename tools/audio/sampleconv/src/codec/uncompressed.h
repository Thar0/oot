#ifndef CODEC_UNCOMPRESSED_H
#define CODEC_UNCOMPRESSED_H

int
pcm16_dec(struct container_data *ctnr, const struct codec_spec *codec, const struct enc_dec_opts *opts);

int
pcm16_enc(struct container_data *ctnr, const struct codec_spec *codec, const struct enc_dec_opts *opts);

int
pcm16_enc_dec(struct container_data *ctnr, const struct codec_spec *codec, const struct enc_dec_opts *opts);

#endif
