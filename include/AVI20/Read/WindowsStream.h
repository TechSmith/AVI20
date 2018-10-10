#pragma once

#include <AVI20/AVI20Types.h>
#include <AVI20/Read/IStream.h>

struct IStream;

NAMESPACE_AVI20_READ_BEGIN

class WindowsStream : public IStream
{
public:
   WindowsStream( ::IStream *stream ) :_Stream( stream ), _CachedSize( -1 ), _CachedPos( 0ULL ) {}

   bool IsNULL() const override;

   void Read( uint32_t& result ) override { result = read<uint32_t>(); }
   void Read( AVI20::AVISTDINDEX& result ) override { result = read <AVI20::AVISTDINDEX>(); }
   void Read( AVI20::AVISTDINDEX_ENTRY& result ) override { result = read<AVI20::AVISTDINDEX_ENTRY>(); }
   void Read( AVI20::AVISUPERINDEX& result ) override { result = read<AVI20::AVISUPERINDEX>(); }
   void Read( AVI20::AVISUPERINDEXENTRY& result ) override { result = read<AVI20::AVISUPERINDEXENTRY>(); }
   void Read( AVI20::BITMAPINFOHEADER& result ) override { result = read<AVI20::BITMAPINFOHEADER>(); }
   void Read( AVI20::MainHeader& result ) override { result = read<AVI20::MainHeader>(); }
   void Read( AVI20::MediaStreamHeader& result ) override { result = read<AVI20::MediaStreamHeader>(); }
   void Read( AVI20::WAVEFORMATEX& result ) override { result = read<AVI20::WAVEFORMATEX>(); }

   bool Read( uint8_t* dest, uint64_t size ) override;
   uint64_t Pos() override;
   void SetPos( uint64_t pos ) override;
   void SetPosToEnd() override;
   uint64_t Size() override;
   bool IsGood() const override;
   void Rewind() override;

private:
   template <typename T>
   T read()
   {
      T t;
      Read( (uint8_t *)&t, sizeof( T ) );
      return t;
   }

   ::IStream*    _Stream;  // doesn't own
   int64_t       _CachedSize;
   uint64_t      _CachedPos;
};
NAMESPACE_AVI20_READ_END
