#include <AVI20/Write/StreamW.h>
#include <AVI20/Write/StreamPosRestorerW.h>
#include <algorithm>

NAMESPACE_AVI20_WRITE_BEGIN

uint64_t Stream::Size()
{
   StreamPosRestorer restorer( *this );
   SetPosToEnd();
   return Pos();
}

void Stream::WriteZeros( uint64_t size )
{
   const uint8_t zeros[1024] = {0};
   uint64_t bytesLeft = size;
   while ( bytesLeft > 0 )
   {
      uint64_t ct = std::min<uint64_t>( sizeof(zeros), bytesLeft );
      Write( zeros, ct );
      bytesLeft -= ct;
   }
}

NAMESPACE_AVI20_WRITE_END
