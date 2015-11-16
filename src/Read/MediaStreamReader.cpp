#include <AVI20/Read/MediaStreamReader.h>
#include <AVI20/Read/MediaStreamReaderImpl.h>

NAMESPACE_AVI20_READ_BEGIN

MediaStreamReader::MediaStreamReader( MediaStreamReaderImpl* impl )
   : _Impl( impl )
{
}

MediaStreamReader::~MediaStreamReader(){}

bool            MediaStreamReader::IsValid() const                                           { return _Impl != 0; }

uint32_t        MediaStreamReader::NumFrames() const                                         { return IsValid() ? _Impl->NumFrames() : 0; }
uint32_t        MediaStreamReader::FrameSize( uint32_t frameIndex ) const                    { return IsValid() ? _Impl->FrameSize( frameIndex ) : 0; }  
uint64_t        MediaStreamReader::FrameStreamPos( uint32_t frameIndex ) const               { return IsValid() ? _Impl->FrameStreamPos( frameIndex ) : 0; }  
bool            MediaStreamReader::ReadFrameData( uint32_t frameIndex, uint8_t* dest ) const { return IsValid() ? _Impl->ReadFrameData( frameIndex, dest ) : false; }  
Buffer*         MediaStreamReader::ReadBuffer( uint32_t frameIndex )                         { return IsValid() ? _Impl->ReadBuffer( frameIndex ) : NULL; }
uint64_t        MediaStreamReader::TotalBytes() const                                        { return IsValid() ? _Impl->TotalBytes() : 0; }
void            MediaStreamReader::DeleteBuffer( Buffer*& buffer )                           { return MediaStreamReaderImpl::DeleteBuffer( buffer ); }
MediaStreamInfo MediaStreamReader::StreamInfo() const                                        { return IsValid() ? _Impl->StreamInfo() : MediaStreamInfo(); }
bool            MediaStreamReader::IsKeyframe( uint32_t frameIndex ) const                   { return IsValid() ? _Impl->IsKeyframe( frameIndex ) : false; }
uint32_t        MediaStreamReader::KeyframeAtOrBefore( uint32_t frameIndex ) const           { return IsValid() ? _Impl->KeyframeAtOrBefore( frameIndex ) : 0; }
double          MediaStreamReader::Duration() const                                          { return IsValid() ? _Impl->Duration() : 0; }

uint32_t        MediaStreamReader::ReadBytes( uint64_t mediaByteOffset, uint32_t numBytes, uint8_t* dest ) const { return IsValid() ? _Impl->ReadBytes( mediaByteOffset, numBytes, dest ) : 0; }  

NAMESPACE_AVI20_READ_END