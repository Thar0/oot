#include "attributes.h"
#include "z64audio.h"

// Symbol definition

extern AudioTable gSequenceTable;
#pragma weak gSequenceTable = sSequenceTableHeader

// Externs for table

#define DEFINE_SEQUENCE(name, seqId, storageMedium, cachePolicy, seqFlags, seqPlayer) \
    extern u8 name##_Start[];                                                         \
    extern u8 name##_Size[];
#define DEFINE_SEQUENCE_PTR(seqIdReal, seqId, storageMediumReal, cachePolicyReal, seqFlags, seqPlayer) /*empty*/

#include "tables/sequence_table.h"

#undef DEFINE_SEQUENCE
#undef DEFINE_SEQUENCE_PTR

// Table header

NO_REORDER AudioTableHeader sSequenceTableHeader = {
// The table contains the number of sequences, count them with the preprocessor
#define DEFINE_SEQUENCE(name, seqId, storageMedium, cachePolicy, seqFlags, seqPlayer) 1 +
#define DEFINE_SEQUENCE_PTR(seqIdReal, seqId, storageMediumReal, cachePolicyReal, seqFlags, seqPlayer) 1 +

#include "tables/sequence_table.h"

#undef DEFINE_SEQUENCE
#undef DEFINE_SEQUENCE_PTR
    0,

    0,
    0x00000000,
    { 0, 0, 0, 0, 0, 0, 0, 0 },
};

// Table body

NO_REORDER AudioTableEntry sSequenceTableEntries[] = {
#define DEFINE_SEQUENCE(name, seqId, storageMedium, cachePolicy, seqFlags, seqPlayer) \
    { (u32)name##_Start, (u32)name##_Size, (storageMedium), (cachePolicy), 0, 0, 0 },
#define DEFINE_SEQUENCE_PTR(seqIdReal, seqId, storageMediumReal, cachePolicyReal, seqFlags, seqPlayer) \
    { (seqIdReal), 0, (storageMediumReal), (cachePolicyReal), 0, 0, 0 },

#include "tables/sequence_table.h"

#undef DEFINE_SEQUENCE
#undef DEFINE_SEQUENCE_PTR
};
