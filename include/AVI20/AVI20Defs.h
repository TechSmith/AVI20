#pragma once

#include <AVI20/Namespace.h>
#include <stdint.h>

NAMESPACE_AVI20_BEGIN

typedef uint8_t  BYTE;
typedef int32_t  LONG;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef uint32_t FOURCC;
typedef uint64_t DWORDLONG;

#define REVERSE_4_BYTES( x ) ((((x)&0xFF000000)>>24) \
                            | (((x)&0x00FF0000)>> 8) \
                            | (((x)&0x0000FF00)<< 8) \
                            | (((x)&0x000000FF)<<24))
#ifdef FCC
#undef FCC
#undef streamtypeVIDEO
#undef streamtypeAUDIO
#undef streamtypeMIDI
#undef streamtypeTEXT
#undef WAVEFORMAT_PCM
#undef RIFFROUND
#endif
#define FCC( x ) REVERSE_4_BYTES( x )
#define FCC2( x, n ) AVI20::FCCn( FCC( x ), (n) )

const DWORD streamtypeVIDEO = FCC('vids');
const DWORD streamtypeAUDIO = FCC('auds');
const DWORD streamtypeMIDI  = FCC('mids');
const DWORD streamtypeTEXT  = FCC('txts');

const WORD WAVEFORMAT_PCM = 1;

#define AVIF_HASINDEX        0x00000010 // Index at end of file?
#define AVIF_MUSTUSEINDEX    0x00000020
#define AVIF_ISINTERLEAVED   0x00000100
#define AVIF_TRUSTCKTYPE     0x00000800 // Use CKType to find key frames
#define AVIF_WASCAPTUREFILE  0x00010000
#define AVIF_COPYRIGHTED     0x00020000

#define RIFFROUND(x) ((x+1)&~1)


NAMESPACE_AVI20_END
