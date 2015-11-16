#include <AVI20/Write/FillOutAVIIndex.h>
#include <AVI20/Write/StreamW.h>
#include <AVI20/Read/Stream.h>
#include <AVI20/Read/MediaStreamInfo.h>
#include <AVI20/Read/FillAVIIndexInfo.h>
#include <AVI20/AVI20Types.h>
#include <AVI20/Utl.h>
#include <vector>
#include <fstream>
#include <cassert>


NAMESPACE_AVI20_WRITE_BEGIN

namespace
{

void WriteUpdatedChunkSize( Stream& stream, const Read::ChunkHeader& ch )
{
   stream.SetPos( ch.startPos+4 );
   stream.Write( ch.size );
}




void FillOutAVIIndex( Write::Stream& stream, const Read::FillAVIIndexInfo& info )
{
   // fix chunk sizes (for 'AVI '/'AVIX' and 'movi')
   if ( !info._AVIchs.empty() )
   WriteUpdatedChunkSize( stream, info._AVIchs.back() );
   for ( int i = 0; i < (int) info._MOVIchs.size(); i++ )
      WriteUpdatedChunkSize( stream, info._MOVIchs[i] );

   // update 'avih' microseconds per frame
   {
      stream.SetPos( info._AVIHch.StartDataPos() + offsetof(MainHeader,dwMicroSecPerFrame) );
      stream.Write( DWORD( info.FPS() ? 1000000/info.FPS() : 0 ) );
   }

   // update 'avih' num streams
   {
      stream.SetPos( info._AVIHch.StartDataPos() + offsetof(MainHeader,dwStreams) );
      stream.Write( DWORD( info._StreamInfos.size() ) );
   }

   // update 'avih' dimensions
   {
      BITMAPINFOHEADER bih = info.BitmapInfo();
      stream.SetPos( info._AVIHch.StartDataPos() + offsetof(MainHeader,dwWidth) );
      stream.Write( DWORD( bih.biWidth ) );
      stream.Write( DWORD( bih.biHeight ) );
   }

   // update 'avih' frame count (= #video frames in first RIFF)
   if ( info.FirstVideoStreamIdx() >= 0 )
   {
      DWORD videoFramesInFirstRIFF = info.CalcStreamLength( info.FirstVideoStreamIdx(), true/*first RIFF only*/ );
      stream.SetPos( info._AVIHch.StartDataPos() + offsetof(MainHeader,dwTotalFrames) );
      stream.Write( videoFramesInFirstRIFF );
   }

   // update 'avih' suggested buffer size (use size of first frame)
   if ( info.FirstVideoStreamIdx() >= 0 && !info._StreamInfos[info.FirstVideoStreamIdx()].indexes[0].entries.empty() )
   {
      uint32_t suggestedSize = info._StreamInfos[info.FirstVideoStreamIdx()].indexes[0].entries[0].Size() + 8;
      stream.SetPos( info._AVIHch.StartDataPos() + offsetof(MainHeader,dwSuggestedBufferSize) );
      stream.Write( suggestedSize );
   }

   // for each stream, update 'strl' dwLength member
   for ( int streamIdx = 0; streamIdx < (int)info._StreamInfos.size(); streamIdx++ )
   {
      const Read::AVIStreamInfo& streamInfo = info._StreamInfos[streamIdx];
      stream.SetPos( streamInfo.STRHch.StartDataPos() + offsetof(MediaStreamHeader,dwLength) );
      //DWORD lengthInFirstRIFF = info.CalcStreamLength( streamIdx, true/*first RIFF only*/ );
      DWORD lengthInFirstRIFF = info.CalcStreamLength( streamIdx, false/*all RIFFs*/ ); // using all riffs is against the spec, but DirectShow (and others?) expect it this way
      stream.Write( lengthInFirstRIFF );
   }

   // for each stream, update 'strl' dwSuggestedBufferSize member
   for ( int streamIdx = 0; streamIdx < (int)info._StreamInfos.size(); streamIdx++ )
   {
      const Read::AVIStreamInfo& streamInfo = info._StreamInfos[streamIdx];
      stream.SetPos( streamInfo.STRHch.StartDataPos() + offsetof(MediaStreamHeader,dwSuggestedBufferSize) );
      DWORD lengthInFirstRIFF = streamInfo.largestFrameSize() + 8;
      stream.Write( lengthInFirstRIFF );
   }

   // update 'dmlh' dwTotalLength
   {
      stream.SetPos( info._DMLHch.StartDataPos() );
      stream.Write( info.TotalLength() );
   }

   assert( stream.IsGood() );

   // fill 'indx'
   for ( int streamIdx = 0; streamIdx < (int)info._StreamInfos.size(); streamIdx++ )
   {
      const Read::AVIStreamInfo& streamInfo = info._StreamInfos[streamIdx];
      stream.SetPos( streamInfo.streamInfo.indxChunk.StartDataPos() );
      bool isAudio = streamInfo.streamInfo.IsAudio();

      AVISUPERINDEX superindex = { 0 };
      superindex.wLongsPerEntry = 4;
      superindex.bIndexSubType = 0;                         // frame index
      superindex.bIndexType = 0;                            // AVI_INDEX_OF_INDEXES;
      superindex.nEntriesInUse = streamInfo.indexes.size(); // offset of next unused entry in aIndex
      superindex.dwChunkId = streamInfo.frameFCC( true/*keyframe*/ );
      stream.Write( superindex );

      // ~2014 slots available
      int slotsAvailable = ( streamInfo.streamInfo.indxChunk.size - sizeof(AVISUPERINDEX) ) / sizeof(AVISUPERINDEXENTRY);
      for ( int i = 0; i < (int)streamInfo.indexes.size() && i < slotsAvailable; i++ )
      {
         AVISUPERINDEXENTRY superIndexEntry;
         superIndexEntry.qwOffset   = streamInfo.indexes[i].indexChunk.startPos;
         superIndexEntry.dwSize     = streamInfo.indexes[i].indexChunk.size + 8;
         superIndexEntry.dwDuration = isAudio ? streamInfo.indexes[i].SizeOfEntries() / streamInfo.streamInfo.audio.BlockAlign()
                                              : streamInfo.indexes[i].entries.size();
         stream.Write( superIndexEntry );
      }
      bool error = (int)streamInfo.indexes.size() >= slotsAvailable;
      assert( !error );
   }

   // fill 'ix00'
   for ( int streamIdx = 0; streamIdx < (int)info._StreamInfos.size(); streamIdx++ )
   {
      const Read::AVIStreamInfo& streamInfo = info._StreamInfos[streamIdx];
      for ( int i = 0; i < (int)streamInfo.indexes.size(); i++ )
      {
         const Read::Index& index = streamInfo.indexes[i];
         stream.SetPos( index.indexChunk.StartDataPos() );

         AVISTDINDEX stdIndex;
         stdIndex.wLongsPerEntry    = 2;
         stdIndex.bIndexSubType     = 0;
         stdIndex.bIndexType        = 1;        /*AVI_INDEX_OF_CHUNKS*/
         stdIndex.nEntriesInUse     = index.entries.size();     // offset of next unused entry in aIndex
         stdIndex.dwChunkId         = streamInfo.frameFCC( true/*keyframe*/ );
         stdIndex.qwBaseOffset      = index.basePos;
         stdIndex.dwReserved_3      = 0;
         stream.Write( stdIndex );

         // ~4028
         int slotsAvailable = ( index.indexChunk.size - sizeof(AVISTDINDEX) ) / sizeof(AVISTDINDEX_ENTRY);

         for ( int k = 0; k < (int)index.entries.size() && k < slotsAvailable; k++ )
            stream.Write( index.entries[k] );

         bool error = (int)index.entries.size() > slotsAvailable;
         assert( !error );
      }
   }

   // fill 'idx1'
   if ( info._IDX1ch.IsValid() && !info._MOVIchs.empty() )
   {
      bool error = false;

      stream.SetPos( info._IDX1ch.StartDataPos() );
      int numSlotsLeft = info._IDX1ch.size / sizeof(AVIINDEXENTRY);
      for ( int streamIdx = 0; streamIdx < (int)info._StreamInfos.size(); streamIdx++ )
      {
         const Read::AVIStreamInfo& streamInfo = info._StreamInfos[streamIdx];
         for ( int i = 0; i < (int)streamInfo.indexes.size(); i++ )
         {
            const Read::Index& index = streamInfo.indexes[i];
            bool inFirstRiff = index.indexChunk.StartDataPos() < info._AVIchs[0].EndPos();
            if ( !inFirstRiff )
               continue;
            for ( int k = 0; k < (int) index.entries.size(); k++ )
            {
               if ( numSlotsLeft <= 0 )
               {
                  error = true;
                  continue;
               }
               AVIINDEXENTRY indexEntry;
               indexEntry.ckid          = streamInfo.frameFCC( index.entries[k].IsKeyframe() );
               indexEntry.dwFlags       = index.entries[k].IsKeyframe() ? AVIIF_KEYFRAME : 0;
               indexEntry.dwChunkOffset = DWORD( index.basePos + index.entries[k].Offset() - 8 );
               //indexEntry.dwChunkOffset = DWORD( index.basePos + index.entries[k].Offset() - 8 - info._MOVIchs[0].StartDataPos() );
               indexEntry.dwChunkLength = index.entries[k].Size();
               stream.Write( indexEntry );
               numSlotsLeft--;
            }
         }
      }

      assert( !error );
   }
}

} // end anonymous namespace

void FillOutAVIIndex( const std::string& filename )
{
   Read::FillAVIIndexInfo info;
   info.InitFrom( filename );

   {
      std::fstream f( filename.c_str(), std::ios::in|std::ios::out|std::ios::binary );
      Stream stream( &f );
      FillOutAVIIndex( stream, info );
   }
}

void FillOutAVIIndex( Write::Stream& stream )
{
   Read::FillAVIIndexInfo info;
   Read::Stream readStream( stream );
   info.InitFrom( readStream );
   FillOutAVIIndex( stream, info );
}

NAMESPACE_AVI20_WRITE_END
