/***************************************************************************//**
*
* @file          video_marco.h
* @brief         video macro definition for all platform
* @authors       Chao Zhou
* @copyright     (c) 2020, Chao Zhou <czhou2@qq.com>  All rights reserved
*
*******************************************************************************/
#ifndef _video_marco_h_
#define _video_marco_h_

#undef BIT_SCAN_FORWARD_32
#define BIT_SCAN_FORWARD_32(VALUE, POS) \
{                                       \
    UINT32  bitsLeft;                   \
    SINT32  count;                      \
                                        \
    bitsLeft = VALUE & 0xFFFFFFFF;      \
    count    = 0;                       \
                                        \
    if (bitsLeft != 0)                  \
    {                                   \
        count = 32;                     \
        while (bitsLeft != 0)           \
        {                               \
            count--;                    \
            bitsLeft <<= 1;             \
        }                               \
    }                                   \
                                        \
    POS = count;                        \
                                        \
}

#undef BIT_SCAN_FORWARD_64
#define BIT_SCAN_FORWARD_64(VALUE, POS) \
{                                       \
    UINT64  bitsLeft;                   \
    SINT32  count;                      \
                                        \
    bitsLeft = VALUE;                   \
    count    = 0;                       \
                                        \
    if (bitsLeft != 0)                  \
    {                                   \
        count = 64;                     \
        while (bitsLeft != 0)           \
        {                               \
            count--;                    \
            bitsLeft <<= 1;             \
        }                               \
    }                                   \
                                        \
    POS = count;                        \
                                        \
}

#undef BIT_SCAN_REVERSE_32
#define BIT_SCAN_REVERSE_32(VALUE, POS) \
{                                       \
    UINT32  bitsLeft;                   \
    SINT32  count;                      \
                                        \
    bitsLeft = VALUE & 0xFFFFFFFF;      \
    count    = 0;                       \
                                        \
    if (bitsLeft != 0)                  \
    {                                   \
        count = -1;                     \
        while (bitsLeft != 0)           \
        {                               \
            count++;                    \
            bitsLeft >>= 1;             \
        }                               \
    }                                   \
                                        \
    POS = count;                        \
                                        \
}

#undef BIT_SCAN_REVERSE_64
#define BIT_SCAN_REVERSE_64(VALUE, POS) \
{                                       \
    UINT64  bitsLeft;                   \
    SINT32  count;                      \
                                        \
    bitsLeft = VALUE;                   \
    count    = 0;                       \
                                        \
    if (bitsLeft != 0)                  \
    {                                   \
        count = -1;                     \
        while (bitsLeft != 0)           \
        {                               \
            count++;                    \
            bitsLeft >>= 1;             \
        }                               \
    }                                   \
                                        \
    POS = count;                        \
                                        \
}

#if defined(_WIN32)

#undef BIT_SCAN_FORWARD_32
#define BIT_SCAN_FORWARD_32(VALUE, POS) \
{                                       \
    _BitScanForward(&POS, VALUE);       \
}
#if defined(_WIN64)
#undef BIT_SCAN_FORWARD_64
#define BIT_SCAN_FORWARD_64(VALUE, POS) \
{                                       \
    _BitScanForward64(&POS, VALUE);     \
}
#else
#undef BIT_SCAN_FORWARD_64
#define BIT_SCAN_FORWARD_64(VALUE, POS) \
{                                       \
    UINT32  valueHi;                    \
    UINT32  valueLo;                    \
                                        \
    valueHi = (VALUE >> 32);            \
    valueLo = (VALUE & 0xFFFFFFFF);     \
                                        \
                                        \
    if (valueLo == 0)                   \
    {                                   \
        _BitScanForward(&POS, valueHi); \
        POS += 32;                      \
    }                                   \
    else                                \
    {                                   \
        _BitScanForward(&POS, valueLo); \
    }                                   \
}
#endif

#undef BIT_SCAN_REVERSE_32
#define BIT_SCAN_REVERSE_32(VALUE, POS) \
{                                       \
    _BitScanReverse(&POS, VALUE);       \
}
#if defined(_WIN64)
#undef BIT_SCAN_REVERSE_64
#define BIT_SCAN_REVERSE_64(VALUE, POS) \
{                                       \
    _BitScanReverse64(&POS, VALUE);     \
}
#else
#undef BIT_SCAN_REVERSE_64
#define BIT_SCAN_REVERSE_64(VALUE, POS) \
{                                       \
    UINT32  valueHi;                    \
    UINT32  valueLo;                    \
                                        \
    valueHi = (VALUE >> 32);            \
    valueLo = (VALUE & 0xFFFFFFFF);     \
                                        \
                                        \
    if (valueHi != 0)                   \
    {                                   \
        _BitScanReverse(&POS, valueHi); \
        POS += 32;                      \
    }                                   \
    else                                \
    {                                   \
        _BitScanReverse(&POS, valueLo); \
    }                                   \
}
#endif
#elif defined(__linux__) || defined(__APPLE__)
#undef BIT_SCAN_FORWARD_32
#define BIT_SCAN_FORWARD_32(VALUE, POS) \
{                                       \
    (POS) = __builtin_ctz(VALUE);       \
}

#undef BIT_SCAN_FORWARD_64
#define BIT_SCAN_FORWARD_64(VALUE, POS) \
{                                       \
    (POS) = __builtin_ctzll(VALUE);     \
}

#undef BIT_SCAN_REVERSE_32
#define BIT_SCAN_REVERSE_32(VALUE, POS) \
{                                       \
    (POS) = 31 - __builtin_clz(VALUE);  \
}

#undef BIT_SCAN_REVERSE_64
#define BIT_SCAN_REVERSE_64(VALUE, POS) \
{                                       \
    (POS) = 63 - __builtin_clzll(VALUE);\
}
#else
#endif

#endif
