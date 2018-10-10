#pragma once

#include <AVI20/Namespace.h>
#include <AVI20/AVI20Defs.h>
#include <AVI20/Read/ChunkHeader.h>

NAMESPACE_AVI20_BEGIN
class Buffer;
NAMESPACE_AVI20_END

NAMESPACE_AVI20_READ_BEGIN

class IStream;
struct Type2Index;

class FrameIndex
{
public:
   FrameIndex( IStream& stream, const ChunkHeader& indxChunk );
   virtual ~FrameIndex();

   uint32_t       NumFrames() const;
   uint32_t       FrameSize( uint32_t frameIndex ) const;
   uint64_t       FrameStreamPos( uint32_t frameIndex ) const;
   bool           ReadFrameData( uint32_t frameIndex, uint8_t* dest ) const;
   uint32_t       ReadFrameData( uint32_t frameIndex, uint32_t offsetIntoData, uint32_t numBytesRequested, uint8_t* dest ) const;
   Buffer*        ReadBuffer( uint32_t frameIndex );
   static void    DeleteBuffer( Buffer*& buffer );
   bool           IsKeyframe( uint32_t frameIndex ) const;
   uint32_t       KeyframeAtOrBefore( uint32_t frameIndex ) const;



public:
   uint32_t       ReadBytes( uint64_t mediaBytesOffset, uint32_t numBytesRequested, uint8_t* dest );
   uint64_t       TotalMediaBytes() const;
   uint64_t       MediaByteOffsetForFrame( uint32_t frameIndex ) const;
   int32_t        FrameContainingMediaByteOffset( uint64_t mediaByteOffset ) const;

private:
   void Parse();

private:
   IStream&    _Stream;
   ChunkHeader _IndxChunk;
   Type2Index* _Type2Index;
};

NAMESPACE_AVI20_READ_END