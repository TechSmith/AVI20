#include <AVI20/WaveFormatEx.h>
#include <AVI20/Read/Stream.h>
#include <AVI20/Write/StreamW.h>
#include <cassert>

NAMESPACE_AVI20_BEGIN

WaveFormatEx::WaveFormatEx() 
{ 
   memset( &_Format, 0, sizeof(_Format) ); 
}

WaveFormatEx::WaveFormatEx( const WAVEFORMATEX& wfx )
{
   _Format = wfx;
   const uint8_t* p = (const uint8_t*) &wfx + 18;
   _ExtraData.insert( _ExtraData.end(), p, p + wfx.cbSize );
}

WaveFormatEx WaveFormatEx::PCM( int sampleRate, int bitDepth, int numChannels )
{
   WaveFormatEx ret;

   ret._Format.wFormatTag      = 1; /*PCM*/
   ret._Format.nChannels       = numChannels;
   ret._Format.nSamplesPerSec  = sampleRate;
   ret._Format.wBitsPerSample  = bitDepth;
   ret._Format.nBlockAlign     = numChannels * ret._Format.wBitsPerSample / 8;
   ret._Format.nAvgBytesPerSec = ret._Format.nBlockAlign * ret._Format.nSamplesPerSec;
   ret._Format.cbSize          = 0;

   return ret;
}

void WaveFormatEx::InitFromStream( Read::IStream& stream )
{
   stream.Read( _Format );
   _ExtraData.resize( _Format.cbSize );
   if ( !_ExtraData.empty() )
      stream.Read( &_ExtraData[0], _Format.cbSize );
}

WaveFormatEx WaveFormatEx::FromStream( Read::IStream& stream )
{
   WaveFormatEx ret;
   ret.InitFromStream( stream );
   return ret;
}

void WaveFormatEx::WriteToStream( Write::Stream& stream ) const
{
   assert( IsConsistent() );
   stream.Write( _Format );
   if ( !_ExtraData.empty() )
      stream.Write( &_ExtraData[0], _Format.cbSize );
}

void WaveFormatEx::CopyTo( void* buf ) const
{
   memcpy( buf, &_Format, 18 );
   if ( !_ExtraData.empty() )
      memcpy( (uint8_t*) buf + 18, &_ExtraData[0], _ExtraData.size() );
}
   
NAMESPACE_AVI20_END