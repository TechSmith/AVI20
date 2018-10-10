#pragma once
#include <AVI20/Namespace.h>
#include <AVI20/AVI20Types.h>

NAMESPACE_AVI20_READ_BEGIN

class IStream;

struct StreamPosRestorer
{
   StreamPosRestorer( IStream& s );
   ~StreamPosRestorer();
   IStream&  _Stream;
   uint64_t  _Pos;
};

NAMESPACE_AVI20_READ_END