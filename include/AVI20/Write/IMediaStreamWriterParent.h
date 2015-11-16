#pragma once

#include <AVI20/Namespace.h>

NAMESPACE_AVI20_WRITE_BEGIN

class IMediaStreamWriterParent
{
public:
   virtual void OnBeforeFrameWrite( int streamIndex, int frameSize ) = 0;
};

NAMESPACE_AVI20_WRITE_END