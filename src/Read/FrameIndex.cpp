#include <AVI20/Read/FrameIndex.h>
#include <AVI20/AVI20Types.h>
#include <AVI20/Read/Stream.h>
#include <AVI20/Buffer.h>
#include <AVI20/AVI20Types.h>
#include <vector>
#include <algorithm>

#ifdef min
#undef min
#endif

NAMESPACE_AVI20_READ_BEGIN


struct FrameInfo
{
public:
   FrameInfo() {}
   FrameInfo( const AVISTDINDEX_ENTRY& entry, uint64_t offset ) : pos( entry.Offset() + offset ), size( entry.RawSize() ) {}
public:
   uint64_t Offset() const     { return pos; }
   DWORD    Size() const       { return size & AVISTDINDEX_SIZEMASK; }
   bool     IsKeyframe() const { return ( size & AVISTDINDEX_DELTAFRAME ) == 0; }
private:
   uint64_t pos;
   DWORD    size;
};

struct Type2Index
{
public:
   void Read( IStream& stream, uint64_t endPos )
   {
      stream.Read( superIndex );

      if ( superIndex.wLongsPerEntry != 4 )                         Stream::ThrowException();
      if ( superIndex.bIndexType     != 0/*AVI_INDEX_OF_INDEXES*/ ) Stream::ThrowException();
      if ( superIndex.bIndexSubType  != 0 )                         Stream::ThrowException();
      uint64_t sizeOfIndexes = (uint64_t)superIndex.nEntriesInUse*superIndex.wLongsPerEntry*4;
      if ( stream.Pos() + sizeOfIndexes > endPos )                  Stream::ThrowException();

      for ( int i = 0; i < (int)superIndex.nEntriesInUse; i++ )
      {
         AVISUPERINDEXENTRY entry;
         stream.Read( entry );
         indexes.push_back( entry );
      }

      for ( int idx = 0; idx < (int) indexes.size(); idx++ )
      {
         stream.SetPos( indexes[idx].qwOffset + 8 );
         AVISTDINDEX stdIndex;
         stream.Read( stdIndex );
         for ( int i = 0; i < (int) stdIndex.nEntriesInUse; i++ )
         {
            AVISTDINDEX_ENTRY entry;
            stream.Read( entry );
            frames.push_back( FrameInfo( entry, stdIndex.qwBaseOffset ) );
         }
      }

      uint64_t totalMediaBytes = 0;
      for ( int i = 0; i < (int) frames.size(); i++ )
      {
         mediaByteOffsetForFrame.push_back( totalMediaBytes );
         totalMediaBytes += frames[i].Size();
      }
   }

   uint64_t TotalMediaBytes() const
   {
      if ( mediaByteOffsetForFrame.empty() )
         return 0;

      return mediaByteOffsetForFrame.back() + frames.back().Size();
   }

   uint64_t MediaByteOffsetForFrame( uint32_t frameIndex ) const
   {
      if ( frameIndex < 0 )
         return 0;
      if ( frameIndex >= mediaByteOffsetForFrame.size() )
         return TotalMediaBytes();
      return mediaByteOffsetForFrame[frameIndex];
   }

   int32_t FrameContainingMediaByteOffset( uint64_t mediaByteOffset ) const
   {
      return std::upper_bound( mediaByteOffsetForFrame.begin(), mediaByteOffsetForFrame.end(), mediaByteOffset ) - mediaByteOffsetForFrame.begin() - 1;
   }

public:
   AVISUPERINDEX                    superIndex;
   std::vector<AVISUPERINDEXENTRY>  indexes;
   std::vector<FrameInfo>           frames;
   std::vector<uint64_t>            mediaByteOffsetForFrame;
};



FrameIndex::FrameIndex( IStream& stream, const ChunkHeader& indxChunk )
   : _Stream( stream )
   , _IndxChunk( indxChunk )
{
   _Type2Index = new Type2Index;
   Parse();
}

FrameIndex::~FrameIndex()
{
   delete _Type2Index;
}

void FrameIndex::Parse()
{
   _Stream.Rewind();
   _Stream.SetPos( _IndxChunk.StartDataPos() );
   _Type2Index->Read( _Stream, _IndxChunk.EndPos() );
}

uint32_t FrameIndex::NumFrames() const
{
   return _Type2Index->frames.size();
}

uint32_t FrameIndex::FrameSize( uint32_t frameIndex ) const
{
   if ( frameIndex >= NumFrames() )
      return 0;
   return _Type2Index->frames[frameIndex].Size();
}

uint64_t FrameIndex::FrameStreamPos( uint32_t frameIndex ) const
{
   if ( frameIndex >= NumFrames() )
      return 0;
   return _Type2Index->frames[frameIndex].Offset();
}

bool FrameIndex::ReadFrameData( uint32_t frameIndex, uint8_t* dest ) const
{
   if ( frameIndex >= NumFrames() )
      return false;
   uint32_t size = FrameSize( frameIndex );
   uint64_t pos  = FrameStreamPos( frameIndex );

   _Stream.SetPos( pos );
   return _Stream.Read( dest, size );
}

uint32_t FrameIndex::ReadFrameData( uint32_t frameIndex, uint32_t offsetIntoData, uint32_t numBytesRequested, uint8_t* dest ) const
{
   if ( frameIndex >= NumFrames() )
      return 0;
   if ( offsetIntoData >= FrameSize( frameIndex ) )
      return 0;
   uint32_t size = std::min( FrameSize( frameIndex ) - offsetIntoData, numBytesRequested );
   uint64_t pos  = FrameStreamPos( frameIndex ) + offsetIntoData;

   _Stream.SetPos( pos );
   if ( !_Stream.Read( dest, size ) )
      return 0;
   return size;
}

Buffer* FrameIndex::ReadBuffer( uint32_t frameIndex )
{
   if ( frameIndex >= NumFrames() )
      return 0;
   Buffer* ret = new Buffer( FrameSize( frameIndex ) );
   if ( !ReadFrameData( frameIndex, ret->Data() ) )
      DeleteBuffer( ret );
   return ret;
}

void FrameIndex::DeleteBuffer( Buffer*& buffer )
{
   delete buffer;
   buffer = 0;
}

bool FrameIndex::IsKeyframe( uint32_t frameIndex ) const
{
   if ( frameIndex >= NumFrames() )
      return false;
   return _Type2Index->frames[frameIndex].IsKeyframe();
}

uint32_t FrameIndex::KeyframeAtOrBefore( uint32_t frameIndex ) const
{
   for ( uint32_t i = frameIndex; i > 0; i-- )
      if ( IsKeyframe( i ) )
         return i;
   return 0;
}

uint64_t FrameIndex::TotalMediaBytes() const
{
   return _Type2Index ? _Type2Index->TotalMediaBytes() : 0;
}

uint64_t FrameIndex::MediaByteOffsetForFrame( uint32_t frameIndex ) const
{
   if ( !_Type2Index )
      return 0;
   return _Type2Index->MediaByteOffsetForFrame( frameIndex );
}

int32_t FrameIndex::FrameContainingMediaByteOffset( uint64_t mediaByteOffset ) const
{
   if ( !_Type2Index )
      return -1;
   return _Type2Index->FrameContainingMediaByteOffset( mediaByteOffset );
}

uint32_t FrameIndex::ReadBytes( uint64_t mediaBytesOffset, uint32_t numBytesRequested, uint8_t* dest )
{
   int32_t frameIndex = FrameContainingMediaByteOffset( mediaBytesOffset );
   if ( frameIndex < 0 )
      return 0;

   int32_t offsetIntoFrame = int32_t( mediaBytesOffset - MediaByteOffsetForFrame( frameIndex ) );

   uint32_t bytesRead = 0;
   while ( bytesRead < numBytesRequested )
   {
      uint32_t newBytesRead = ReadFrameData( frameIndex, offsetIntoFrame, numBytesRequested-bytesRead, dest+bytesRead );
      bytesRead += newBytesRead;
      if ( newBytesRead == 0 )
         break;
      frameIndex++;
      offsetIntoFrame = 0;
   }

   return bytesRead;
}

NAMESPACE_AVI20_READ_END
