#include <AVI20/Write/StreamPosRestorerW.h>
#include <AVI20/Write/StreamW.h>

NAMESPACE_AVI20_WRITE_BEGIN

StreamPosRestorer::StreamPosRestorer( Stream& s )
   : _Stream( s )
{
   _Pos = _Stream.Pos();
}

StreamPosRestorer::~StreamPosRestorer()
{
   _Stream.SetPos( _Pos );
}

NAMESPACE_AVI20_WRITE_END