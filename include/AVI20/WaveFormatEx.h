#pragma once

#include <AVI20/Namespace.h>
#include <AVI20/AVI20Types.h>
#include <vector>

NAMESPACE_AVI20_READ_BEGIN

class Stream;

NAMESPACE_AVI20_READ_END

NAMESPACE_AVI20_WRITE_BEGIN

class Stream;

NAMESPACE_AVI20_WRITE_END

NAMESPACE_AVI20_BEGIN

#pragma warning( push )
#pragma warning( disable : 4251 ) // warning C4251: std::vector needs to have dll-interface to be used by clients of class
   
class AVI20_API WaveFormatEx
{
public:
   WaveFormatEx();
   WaveFormatEx( const WAVEFORMATEX& wfx );

public:
   static WaveFormatEx PCM( int sampleRate, int bitDepth, int numChannels );

public:
   void InitFromStream( Read::Stream& stream );
   static WaveFormatEx FromStream( Read::Stream& stream );
   void WriteToStream( Write::Stream& stream ) const;

public:
   DWORD TotalSize() const { return sizeof(_Format) + ExtraSize(); }
   DWORD ExtraSize() const { return (DWORD) _Format.cbSize; }
   bool  IsConsistent() const { return _Format.cbSize == _ExtraData.size(); }
   bool  IsPCM() const { return _Format.wFormatTag == 1; }
                          
public:   
   const WAVEFORMATEX& Format() const              { return _Format; }
   const std::vector<uint8_t>& ExtraData() const   { return _ExtraData; }
   void CopyTo( void* buf ) const;

public:
   WORD  FormatTag()      const { return _Format.wFormatTag; }
   WORD  Channels()       const { return _Format.nChannels; }
   DWORD SamplesPerSec()  const { return _Format.nSamplesPerSec; }
   DWORD AvgBytesPerSec() const { return _Format.nAvgBytesPerSec; }
   WORD  BlockAlign()     const { return _Format.nBlockAlign; }
   WORD  BitsPerSample()  const { return _Format.wBitsPerSample; }
   WORD  ExtraDataSize()  const { return _Format.cbSize; }

private:
   WAVEFORMATEX         _Format;
   std::vector<uint8_t> _ExtraData;
};

#pragma warning( pop )

NAMESPACE_AVI20_END