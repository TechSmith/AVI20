#pragma once

#include <AVI20/Namespace.h>
#include <AVI20/AVI20Defs.h>
#include <istream>

NAMESPACE_AVI20_WRITE_BEGIN
class Stream;
NAMESPACE_AVI20_WRITE_END

NAMESPACE_AVI20_READ_BEGIN

class AVI20_API Stream
{
public:
   Stream( std::istream* file = NULL ) : _File( file ), _CachedSize( -1 ) {}
   Stream( Write::Stream& stream );

   bool IsNULL() const                       { return _File == NULL; }
   template<class T> T Read()                { T t; _File->read( (char*) &t, sizeof(t) ); return t; }
   bool Read( uint8_t* dest, uint64_t size ) { _File->read( (char*) dest, size ); return _File->good(); }
   uint64_t Pos()                            { return _File->tellg(); }
   void SetPos( uint64_t pos )               { _File->seekg( pos ); }
   void SetPosToEnd()                        { _File->seekg( 0, std::ios::end ); }
   uint64_t Size();
   bool IsGood() const                       { return _File->good(); }
   void Rewind()                             { _File->clear(); SetPos( 0 ); }

   static void ThrowException()              { throw std::exception(); }

private:
   std::istream* _File; // doesn't own
   int64_t       _CachedSize;
};


NAMESPACE_AVI20_READ_END