#pragma once

#include <AVI20/AVI20Types.h>
#include <AVI20/Read/IStream.h>

#include <istream>

NAMESPACE_AVI20_WRITE_BEGIN
class Stream;
NAMESPACE_AVI20_WRITE_END

NAMESPACE_AVI20_READ_BEGIN

class AVI20_API Stream : public IStream
{
public:
   Stream( std::istream* file = NULL ) : _File( file ), /*_Stream(nullptr),*/ _CachedSize( -1 )/*, _CachedPos( 0ULL)*/ {}
   Stream( Write::Stream& stream );

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
      _File->read( (char *)&t, sizeof( T ) );
      return t;
   }
   std::istream* _File; // doesn't own
   int64_t       _CachedSize;
};

NAMESPACE_AVI20_READ_END