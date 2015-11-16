#pragma once

#include <AVI20/Namespace.h>
#include <AVI20/AVI20Types.h>
#include <AVI20/Read/ChunkHeader.h>
#include <AVI20/Read/MediaStreamInfo.h>
#include <vector>

NAMESPACE_AVI20_READ_BEGIN

struct Index
{
   Index() {}
   Index( const ChunkHeader& indexChunk ) : indexChunk(indexChunk), basePos(indexChunk.StartDataPos()) {}
   ChunkHeader                      indexChunk;
   std::vector<AVISTDINDEX_ENTRY>   entries;
   uint64_t                         basePos;

   void AddEntry( const ChunkHeader& ch, bool isKeyframe )
   {
      if ( entries.empty() )
         basePos = ch.startPos; // imitate DirectShow
      entries.push_back( AVISTDINDEX_ENTRY( DWORD( ch.StartDataPos() - basePos ), ch.size, isKeyframe ) );
   }
   DWORD SizeOfEntries() const
   {
      DWORD ret = 0;
      for ( int i = 0; i < (int)entries.size(); i++ )
         ret += entries[i].Size();
      return ret;
   }
};

struct AVIStreamInfo
{
   AVIStreamInfo( int streamIndex=-1 ) : streamIndex(streamIndex) {}
   int                streamIndex;
   MediaStreamInfo    streamInfo;
   std::vector<Index> indexes;
   ChunkHeader        STRHch;

   FOURCC   frameFCC( bool isKeyframe ) const;
   uint32_t largestFrameSize() const;
};

struct FillAVIIndexInfo
{
   FillAVIIndexInfo() : _DMLHch( ChunkHeader::Invalid() )
                      , _AVIHch( ChunkHeader::Invalid() )
                      , _HDRLch( ChunkHeader::Invalid() )
                      , _IDX1ch( ChunkHeader::Invalid() ) {}
   int FirstVideoStreamIdx() const { for ( int i = 0; i < (int)_StreamInfos.size(); i++ ) if ( _StreamInfos[i].streamInfo.IsVideo() ) return i; return -1; }
   BITMAPINFOHEADER BitmapInfo() const { int vidIdx = FirstVideoStreamIdx(); return vidIdx < 0 ? BITMAPINFOHEADER() : _StreamInfos[vidIdx].streamInfo.video; }
   int FPS() const                     { int vidIdx = FirstVideoStreamIdx(); return vidIdx < 0 ?                  0 : _StreamInfos[vidIdx].streamInfo.header.dwRate / _StreamInfos[vidIdx].streamInfo.header.dwScale; }
   DWORD CalcStreamLength( int streamIdx, bool firstRIFFOnly ) const;
   DWORD TotalLength() const { return CalcStreamLength( FirstVideoStreamIdx(), false/*entire AVI*/ ); }
   std::vector<AVIStreamInfo> _StreamInfos;
   ChunkHeader                _DMLHch;
   ChunkHeader                _AVIHch;
   ChunkHeader                _HDRLch;
   ChunkHeader                _IDX1ch;
   std::vector<ChunkHeader>   _AVIchs;
   std::vector<ChunkHeader>   _MOVIchs;

   void InitFrom( Stream& stream );
   void InitFrom( const std::string& filename );
};


NAMESPACE_AVI20_READ_END