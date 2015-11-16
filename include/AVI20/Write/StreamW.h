#pragma once

#include <AVI20/Namespace.h>
#include <AVI20/AVI20Defs.h>
#include <istream>

NAMESPACE_AVI20_READ_BEGIN
class Stream;
NAMESPACE_AVI20_READ_END


NAMESPACE_AVI20_WRITE_BEGIN

class Stream
{
   friend class Read::Stream;
public:
   Stream( std::iostream* file = NULL ) : _File( file ) {}

   bool IsNULL() const                             { return _File == NULL; }
   template<class T> void Write( const T& t )      { Write( (const uint8_t*) &t, sizeof(t) ); }
   template<class T> void WriteAt( const T& t, const uint64_t& pos ) { uint64_t origPos = Pos(); SetPos( pos ); Write( t ); SetPos( origPos ); }
   void Write( const uint8_t* src, uint64_t size ) { _File->write( (char*) src, size ); }
   void WriteZeros( uint64_t size );
   uint64_t Pos()                                  { return _File->tellp(); }
   void SetPos( uint64_t pos )                     { _File->seekp( pos ); }
   void SetPosToEnd()                              { _File->seekp( 0, std::ios::end ); }
   uint64_t Size();
   bool IsGood() const                             { return _File->good(); }
   void DoClear()                                  { _File->clear(); }

   static void ThrowException()                    { throw std::exception(); }

private:
   std::iostream* _File; // doesn't own
};

NAMESPACE_AVI20_WRITE_END