#pragma once

#include <AVI20/Namespace.h>
#include "MediaStreamInfo.h"

NAMESPACE_AVI20_BEGIN
class Buffer;
NAMESPACE_AVI20_END

NAMESPACE_AVI20_READ_BEGIN

struct MediaStreamInfo;
class IStream;
class FrameIndex;

class MediaStreamReaderImpl
{
public:
   MediaStreamReaderImpl( IStream& stream, const MediaStreamInfo& indxChunk );
   virtual ~MediaStreamReaderImpl();
   void ParseIndex();

   uint32_t        NumFrames() const;
   uint32_t        FrameSize( uint32_t frameIndex ) const;
   uint64_t        FrameStreamPos( uint32_t frameIndex ) const;
   bool            ReadFrameData( uint32_t frameIndex, uint8_t* dest ) const;
   uint32_t        ReadBytes( uint64_t mediaByteOffset, uint32_t numBytes, uint8_t* dest ) const;
   uint64_t        TotalBytes() const;
   Buffer*         ReadBuffer( uint32_t frameIndex );
   static void     DeleteBuffer( Buffer*& buffer );
   MediaStreamInfo StreamInfo() const;
   bool            IsKeyframe( uint32_t frameIndex ) const;
   uint32_t        KeyframeAtOrBefore( uint32_t frameIndex ) const;
   double          Duration() const;

private:
   IStream&        _Stream;
   MediaStreamInfo _StreamInfo;
   FrameIndex*     _FrameIndex;
};

NAMESPACE_AVI20_READ_END