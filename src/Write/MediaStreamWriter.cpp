#include <AVI20/Write/MediaStreamWriter.h>
#include <AVI20/Write/MediaStreamWriterImpl.h>

NAMESPACE_AVI20_WRITE_BEGIN

MediaStreamWriter::MediaStreamWriter( MediaStreamWriterImpl* impl )
{
   _Impl = impl;
}

void MediaStreamWriter::WriteFrame( const void* data, const uint32_t len, bool isKeyframe ) { if ( _Impl ) _Impl->WriteFrame( (const uint8_t*) data, len, isKeyframe ); }

NAMESPACE_AVI20_WRITE_END
