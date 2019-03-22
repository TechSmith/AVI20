#include <AVI20/Read/Stream.h>
#include <AVI20/Read/StreamPosRestorer.h>
#include <AVI20/Write/StreamW.h>

NAMESPACE_AVI20_READ_BEGIN

Stream::Stream( Write::Stream& stream )
   : _File( stream._File )
   , _CachedSize( -1 )
{
}

bool Stream::IsNULL() const
{
   return _File == nullptr;
}

bool Stream::Read( uint8_t* dest, uint64_t size )
{
   _File->read( (char*)dest, size );
   return _File->good();
}

uint64_t Stream::Pos()
{
   return _File->tellg();
}

void Stream::SetPos( uint64_t pos )
{
   _File->seekg( pos );
}

void Stream::SetPosToEnd()
{
   _File->seekg( 0, std::ios::end );
}

uint64_t Stream::Size()
{
   if ( _CachedSize >= 0 )
      return (uint64_t)_CachedSize;

   StreamPosRestorer restorer( *this );
   SetPosToEnd();
   _CachedSize = Pos();
   return _CachedSize;
}

bool Stream::IsGood() const
{
   return _File->good();
}

void Stream::Rewind()
{
   _File->clear();
   SetPos( 0ULL );
}

NAMESPACE_AVI20_READ_END