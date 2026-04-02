#ifndef LEVEL_TABLE_H
#define LEVEL_TABLE_H

// For LEVEL_NAME defines, see level_defines.h.
// Please include this file if you want to use them.

#define STUB_LEVEL(_0, levelenum, _2, _3, _4, _5, _6, _7, _8) levelenum,
#define DEFINE_LEVEL(_0, levelenum, _2, _3, _4, _5, _6, _7, _8, _9, _10) levelenum,

enum LevelNum
{
    LEVEL_NONE,
#include "levels/level_defines.h"
    LEVEL_COUNT,
    LEVEL_MAX = LEVEL_COUNT - 1,
    LEVEL_MIN = LEVEL_NONE + 1
};

#ifndef LEVEL_UNKNOWN_1
#define LEVEL_UNKNOWN_1 0x20
#endif
#ifndef LEVEL_UNKNOWN_2
#define LEVEL_UNKNOWN_2 0x21
#endif
#ifndef LEVEL_UNKNOWN_3
#define LEVEL_UNKNOWN_3 0x22
#endif
#ifndef LEVEL_BBH
#define LEVEL_BBH 0x23
#endif
#ifndef LEVEL_CCM
#define LEVEL_CCM 0x24
#endif
#ifndef LEVEL_CASTLE
#define LEVEL_CASTLE 0x25
#endif
#ifndef LEVEL_HMC
#define LEVEL_HMC 0x26
#endif
#ifndef LEVEL_SSL
#define LEVEL_SSL 0x27
#endif
#ifndef LEVEL_BOB
#define LEVEL_BOB 0x28
#endif
#ifndef LEVEL_SL
#define LEVEL_SL 0x29
#endif
#ifndef LEVEL_WDW
#define LEVEL_WDW 0x2A
#endif
#ifndef LEVEL_JRB
#define LEVEL_JRB 0x2B
#endif
#ifndef LEVEL_THI
#define LEVEL_THI 0x2C
#endif
#ifndef LEVEL_TTC
#define LEVEL_TTC 0x2D
#endif
#ifndef LEVEL_RR
#define LEVEL_RR 0x2E
#endif
#ifndef LEVEL_CASTLE_GROUNDS
#define LEVEL_CASTLE_GROUNDS 0x2F
#endif
#ifndef LEVEL_BITDW
#define LEVEL_BITDW 0x30
#endif
#ifndef LEVEL_VCUTM
#define LEVEL_VCUTM 0x31
#endif
#ifndef LEVEL_BITFS
#define LEVEL_BITFS 0x32
#endif
#ifndef LEVEL_SA
#define LEVEL_SA 0x33
#endif
#ifndef LEVEL_BITS
#define LEVEL_BITS 0x34
#endif
#ifndef LEVEL_LLL
#define LEVEL_LLL 0x35
#endif
#ifndef LEVEL_DDD
#define LEVEL_DDD 0x36
#endif
#ifndef LEVEL_WF
#define LEVEL_WF 0x37
#endif
#ifndef LEVEL_ENDING
#define LEVEL_ENDING 0x38
#endif
#ifndef LEVEL_CASTLE_COURTYARD
#define LEVEL_CASTLE_COURTYARD 0x39
#endif
#ifndef LEVEL_PSS
#define LEVEL_PSS 0x3A
#endif
#ifndef LEVEL_COTMC
#define LEVEL_COTMC 0x3B
#endif
#ifndef LEVEL_TOTWC
#define LEVEL_TOTWC 0x3C
#endif
#ifndef LEVEL_BOWSER_1
#define LEVEL_BOWSER_1 0x3D
#endif
#ifndef LEVEL_WMOTR
#define LEVEL_WMOTR 0x3E
#endif
#ifndef LEVEL_UNKNOWN_32
#define LEVEL_UNKNOWN_32 0x3F
#endif
#ifndef LEVEL_BOWSER_2
#define LEVEL_BOWSER_2 0x40
#endif
#ifndef LEVEL_BOWSER_3
#define LEVEL_BOWSER_3 0x41
#endif
#ifndef LEVEL_UNKNOWN_35
#define LEVEL_UNKNOWN_35 0x42
#endif
#ifndef LEVEL_TTM
#define LEVEL_TTM 0x43
#endif
#ifndef LEVEL_UNKNOWN_37
#define LEVEL_UNKNOWN_37 0x44
#endif
#ifndef LEVEL_UNKNOWN_38
#define LEVEL_UNKNOWN_38 0x45
#endif

#undef STUB_LEVEL
#undef DEFINE_LEVEL

#endif // LEVEL_TABLE_H
