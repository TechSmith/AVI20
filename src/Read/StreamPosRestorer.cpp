#include <AVI20/Read/StreamPosRestorer.h>
#include <AVI20/Read/Stream.h>

NAMESPACE_AVI20_READ_BEGIN

StreamPosRestorer::StreamPosRestorer( Stream& s )
   : _Stream( s )
{
   _Pos = _Stream.Pos();
}

StreamPosRestorer::~StreamPosRestorer()
{
   _Stream.SetPos( _Pos );
}

NAMESPACE_AVI20_READ_END