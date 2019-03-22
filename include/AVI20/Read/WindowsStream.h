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
