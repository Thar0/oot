#include "global.h"

// OS Messages

typedef enum {
    /* 0 */ RDB_CPU_BREAK_MSG,
    /* 1 */ RDB_FAULT_MSG,
    /* 2 */ RDB_SP_BREAK_MSG,
    /* 3 */ RDB_RD_MSG
} RdbEventMsg;
