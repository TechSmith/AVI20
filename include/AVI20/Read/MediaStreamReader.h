#pragma once

#include <AVI20/Namespace.h>
#include <AVI20/AVI20Defs.h>

NAMESPACE_AVI20_BEGIN
class Buffer;
NAMESPACE_AVI20_END

NAMESPACE_AVI20_READ_BEGIN

struct MediaStreamInfo;
class MediaStreamReaderImpl;

class MediaStreamReader
{
public:
   MediaStreamReader( MediaStreamReaderImpl* impl );
   virtual ~MediaStreamReader();

   virtual bool            IsValid() const;

   virtual uint32_t        NumFrames() const;
   virtual uint32_t        FrameSize( uint32_t frameIndex ) const;
   virtual uint64_t        FrameStreamPos( uint32_t frameIndex ) const;
   virtual bool            ReadFrameData( uint32_t frameIndex, uint8_t* dest ) const;
   virtual uint32_t        ReadBytes( uint64_t mediaByteOffset, uint32_t numBytes, uint8_t* dest ) const;
   virtual uint64_t        TotalBytes() const;
   virtual Buffer*         ReadBuffer( uint32_t frameIndex );
   static void             DeleteBuffer( Buffer*& buffer );
   virtual MediaStreamInfo StreamInfo() const;
   virtual bool            IsKeyframe( uint32_t frameIndex ) const;
   virtual uint32_t        KeyframeAtOrBefore( uint32_t frameIndex ) const;
   virtual double          Duration() const;

private:
   MediaStreamReaderImpl* _Impl; // does NOT own
};

NAMESPACE_AVI20_READ_END