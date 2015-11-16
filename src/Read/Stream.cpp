#include <AVI20/Read/Stream.h>
#include <AVI20/Read/StreamPosRestorer.h>
#include <AVI20/Write/StreamW.h>

NAMESPACE_AVI20_READ_BEGIN

Stream::Stream( Write::Stream& stream )
   : _File( stream._File )
   , _CachedSize( -1 )
{
}

uint64_t Stream::Size()
{
   if ( _CachedSize >= 0 )
      return (uint64_t) _CachedSize;
   StreamPosRestorer restorer( *this );
   SetPosToEnd();
   _CachedSize = Pos();
   return _CachedSize;
}

NAMESPACE_AVI20_READ_END
