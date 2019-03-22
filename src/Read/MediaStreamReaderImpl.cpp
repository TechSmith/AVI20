#include <AVI20/Read/MediaStreamReaderImpl.h>
#include <AVI20/Read/FrameIndex.h>
#include <AVI20/Read/MediaStreamInfo.h>

NAMESPACE_AVI20_READ_BEGIN

MediaStreamReaderImpl::MediaStreamReaderImpl( IStream& stream, const MediaStreamInfo& streamInfo )
   : _Stream( stream )
   , _StreamInfo( streamInfo )
{
   _FrameIndex = new FrameIndex( stream, streamInfo.indxChunk );
}

MediaStreamReaderImpl::~MediaStreamReaderImpl()
{
   delete _FrameIndex;
}

uint32_t        MediaStreamReaderImpl::NumFrames() const                                         { return _FrameIndex->NumFrames(); }
uint32_t        MediaStreamReaderImpl::FrameSize( uint32_t frameIndex ) const                    { return _FrameIndex->FrameSize( frameIndex ); }
uint64_t        MediaStreamReaderImpl::FrameStreamPos( uint32_t frameIndex ) const               { return _FrameIndex->FrameStreamPos( frameIndex ); }
bool            MediaStreamReaderImpl::ReadFrameData( uint32_t frameIndex, uint8_t* dest ) const { return _FrameIndex->ReadFrameData( frameIndex, dest ); }
uint64_t        MediaStreamReaderImpl::TotalBytes() const                                        { return _FrameIndex->TotalMediaBytes(); }
Buffer*         MediaStreamReaderImpl::ReadBuffer( uint32_t frameIndex )                         { return _FrameIndex->ReadBuffer( frameIndex ); }
void            MediaStreamReaderImpl::DeleteBuffer( Buffer*& buffer )                           { return FrameIndex::DeleteBuffer( buffer ); }
MediaStreamInfo MediaStreamReaderImpl::StreamInfo() const                                        { return _StreamInfo; }
bool            MediaStreamReaderImpl::IsKeyframe( uint32_t frameIndex ) const                   { return _FrameIndex->IsKeyframe( frameIndex ); }
uint32_t        MediaStreamReaderImpl::KeyframeAtOrBefore( uint32_t frameIndex ) const           { return _FrameIndex->KeyframeAtOrBefore( frameIndex ); }

uint32_t        MediaStreamReaderImpl::ReadBytes( uint64_t mediaByteOffset, uint32_t numBytes, uint8_t* dest ) const { return _FrameIndex->ReadBytes( mediaByteOffset, numBytes, dest ); }

double          MediaStreamReaderImpl::Duration() const
{
   MediaStreamInfo streamInfo = StreamInfo();
   if ( streamInfo.IsVideo() )
      return NumFrames() / streamInfo.VideoFPS();

   if ( streamInfo.IsAudio() )
   {
      if ( streamInfo.audio.FormatTag() == 1 /*WAVE_FORMAT_PCM*/ )
         return TotalBytes() / streamInfo.audio.BlockAlign() / (double)streamInfo.audio.SamplesPerSec();

      return TotalBytes() / (double)streamInfo.audio.AvgBytesPerSec();
   }
   return 0;
}

NAMESPACE_AVI20_READ_END
