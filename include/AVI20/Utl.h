#pragma once

#include <AVI20/Namespace.h>
#include <AVI20/AVI20Defs.h>
#include <string>

NAMESPACE_AVI20_BEGIN

struct WAVEFORMATEX;

std::string    FCCtoString( uint32_t fcc );
uint64_t       RiffRound( uint64_t x );
FOURCC         FCCn( FOURCC fcc, int streamIdx );
int            StreamIdxFromFCC( FOURCC fcc );
WAVEFORMATEX   PCMWFX( int sampleRate, int bitsPerSample, int numChannels );

NAMESPACE_AVI20_END