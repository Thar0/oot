#ifndef SEQUENCE_IDS_H
#define SEQUENCE_IDS_H

// Some sequence commands such as runseq would like sequence names for certain arguments.
// This file facilitates making the sequence enum names available in sequence assembly files.

.set __SEQ_ID_CTR, 0

#define DEFINE_SEQUENCE(name, seqId, storageMedium, cachePolicy, seqFlags, seqPlayer) \
    .internal seqId;                                                                  \
    .set seqId, __SEQ_ID_CTR;                                                         \
    .set __SEQ_ID_CTR, __SEQ_ID_CTR + 1

#define DEFINE_SEQUENCE_PTR(seqIdReal, seqId, storageMediumReal, cachePolicyReal, seqFlags, seqPlayer) \
    .internal seqId;                                                                                   \
    .set seqId, __SEQ_ID_CTR;                                                                          \
    .set __SEQ_ID_CTR, __SEQ_ID_CTR + 1

#include "../tables/sequence_table.h"

.internal __SEQ_ID_CTR

#endif
