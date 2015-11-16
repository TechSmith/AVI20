#include <AVI20/Utl.h>
#include <AVI20/AVI20Types.h>
#include <cassert>
#include <algorithm>


NAMESPACE_AVI20_BEGIN

std::string FCCtoString( uint32_t fcc )
{
   return std::string( (char*)&fcc, (char*)&fcc+4 );
}

uint64_t RiffRound( uint64_t x )
{
   return (x+1) & ~1;
}

namespace
{
   FOURCC hexDigit( int x )
   {
      static char hex[] = "0123456789ABCDEF";
      return hex[x & 0xF];
   }
   FOURCC twoDigitHex( int x )
   {
      assert( x >= 0 && x < 256 );
      return ( hexDigit( (x>>0) & 0xF ) << 8 )
           | ( hexDigit( (x>>4) & 0xF ) << 0 );
   }
   int decimalDigitFromHex( char ch )
   {
      if ( isdigit( ch ) ) return ch - '0';
      if ( ch >= 'A' && ch <= 'F' ) return ch - 'A' + 10;
      if ( ch >= 'a' && ch <= 'f' ) return ch - 'a' + 10;
      return -1;
   }
   int StreamIdxFromTCC( FOURCC tcc ) // 2 digits
   {
      int dig0 = decimalDigitFromHex( (tcc>>8)&0xFF );
      int dig1 = decimalDigitFromHex( (tcc>>0)&0xFF );
      if ( dig0 < 0 || dig1 < 0 )
         return -1;
      return dig1 * 16 + dig0;
   }
}

FOURCC FCCn( FOURCC fcc, int streamIdx )
{
   if ( ( (fcc>> 0) & 0x0000FFFF ) == '##' )
      return ( fcc & ~0x0000FFFF ) | (twoDigitHex( streamIdx )<<0);
   if ( ( (fcc>>16) & 0x0000FFFF ) == '##' )
      return ( fcc & ~0xFFFF0000 ) | (twoDigitHex( streamIdx )<<16);
   assert( false );
   return fcc;
}

int StreamIdxFromFCC( FOURCC fcc )
{
   int idx0 = StreamIdxFromTCC( (fcc>> 0) & 0xFFFF );
   int idx1 = StreamIdxFromTCC( (fcc>>16) & 0xFFFF );
   if ( idx0 < 0 ) return idx1;
   if ( idx1 < 0 ) return idx0;
   return std::min( idx0, idx1 );
}

WAVEFORMATEX PCMWFX( int sampleRate, int bitsPerSample, int numChannels )
{
   WAVEFORMATEX wfx;
   wfx.wFormatTag       = WAVEFORMAT_PCM;
   wfx.nChannels        = numChannels;
   wfx.nSamplesPerSec   = sampleRate;
   wfx.wBitsPerSample   = bitsPerSample;
   wfx.nBlockAlign      = wfx.nChannels * wfx.wBitsPerSample / 8;
   wfx.nAvgBytesPerSec  = wfx.nSamplesPerSec * wfx.nBlockAlign;
   wfx.cbSize           = 0;
   return wfx;
}

NAMESPACE_AVI20_END