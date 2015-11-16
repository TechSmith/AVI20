#pragma once

#include <AVI20/Namespace.h>
#include <AVI20/AVI20Defs.h>

NAMESPACE_AVI20_BEGIN

class FCCParser
{
public:
   FCCParser( AVI20::FOURCC fcc );
   operator AVI20::FOURCC() const { return FOURCC(); }
   AVI20::FOURCC FOURCC() const { return _FCC; }
   int StreamIndex() const;

private:
   AVI20::FOURCC  _FCC;
   int            _StreamIndex;
};

NAMESPACE_AVI20_END