#pragma once

#include <AVI20/Namespace.h>
#include <AVI20/AVI20Defs.h>
#include <AVI20/Read/ChunkHeader.h>

#include <memory>

NAMESPACE_AVI20_BEGIN
class Buffer;
NAMESPACE_AVI20_END

NAMESPACE_AVI20_READ_BEGIN

struct FrameInfo;
class IStream;

class Index
{
public:
   virtual ~Index() {}

   virtual std::shared_ptr<const FrameInfo> FrameAt( uint32_t index ) const = 0;
   virtual uint32_t NumFrames() const = 0;

   virtual void Read( IStream& stream, uint64_t endPos ) = 0;

   virtual uint64_t TotalMediaBytes() const = 0;
   virtual uint64_t MediaByteOffsetForFrame( uint32_t frameIndex ) const = 0;
   virtual int32_t FrameContainingMediaByteOffset( uint64_t mediaByteOffset ) const = 0;
};


class FrameIndex
{
public:
   FrameIndex( IStream& stream, const ChunkHeader& indxChunk, uint64_t moviPos );
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
   IStream&                _Stream;
   ChunkHeader             _IndxChunk;
   std::unique_ptr<Index>  _Index;
};

NAMESPACE_AVI20_READ_END