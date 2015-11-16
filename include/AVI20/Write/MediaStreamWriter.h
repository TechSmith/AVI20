#pragma once

#include <AVI20/Namespace.h>
#include <AVI20/AVI20Types.h>

NAMESPACE_AVI20_WRITE_BEGIN

class MediaStreamWriterImpl;

class MediaStreamWriter
{
public:
   MediaStreamWriter() : _Impl( nullptr ) {}
   MediaStreamWriter( MediaStreamWriterImpl* impl );

   bool IsNull() const { return _Impl == nullptr; }

   void WriteFrame( const void* data, const uint32_t len, bool isKeyframe );

private:
   MediaStreamWriterImpl* _Impl; // does NOT own
};

NAMESPACE_AVI20_WRITE_END