#pragma once
#include <AVI20/Namespace.h>
#include <AVI20/AVI20Types.h>

NAMESPACE_AVI20_WRITE_BEGIN

class Stream;

struct StreamPosRestorer
{
   StreamPosRestorer( Stream& s );
   ~StreamPosRestorer();
   Stream&  _Stream;
   uint64_t _Pos;
};

NAMESPACE_AVI20_WRITE_END