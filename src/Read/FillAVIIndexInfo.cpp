#include <AVI20/Read/FillAVIIndexInfo.h>
#include <AVI20/Read/ParserBase.h>
#include <AVI20/Read/Stream.h>
#include <AVI20/Utl.h>
#include <AVI20/FCCParser.h>
#include <algorithm>
#include <cassert>
#include <fstream>

NAMESPACE_AVI20_READ_BEGIN

FOURCC AVIStreamInfo::frameFCC( bool isKeyframe ) const
{
   assert( streamInfo.IsVideo() || streamInfo.IsAudio() );
   return streamInfo.IsVideo() ? isKeyframe ? FCC2('##db',streamIndex)
                                            : FCC2('##dc',streamIndex)
                                 : FCC2('##wb',streamIndex);
}

uint32_t AVIStreamInfo::largestFrameSize() const
{
   uint32_t ret = 0;
   for ( int i = 0; i < (int) indexes.size(); i++ )
   for ( int e = 0; e < (int) indexes[i].entries.size(); e++ )
      ret = std::max( ret, indexes[i].entries[e].Size() );
   return ret;
}

class CaptureAVIParser : public ParserBase
{
public:
   CaptureAVIParser( FillAVIIndexInfo& info ) : _Info(info) {}
   virtual void OnGotChunk( const ChunkHeader& ch )
   {
      FCCParser fcc( ch.fcc );

      // per file
      if ( fcc == FCC('dmlh') ) _Info._DMLHch = ch;
      if ( fcc == FCC('hdrl') ) _Info._HDRLch = ch;
      if ( fcc == FCC('avih') ) _Info._AVIHch = ch;
      if ( fcc == FCC('idx1') ) _Info._IDX1ch = ch;

      // per stream
      if ( fcc == FCC('strl') ) { int idx = (int)_Info._StreamInfos.size(); _Info._StreamInfos.push_back( AVIStreamInfo( idx ) ); }
      if ( fcc == FCC('strh') ) { _Info._StreamInfos.back().streamInfo.header = _Stream->Read<MediaStreamHeader>(); _Info._StreamInfos.back().STRHch = ch; }
      if ( fcc == FCC('indx') ) _Info._StreamInfos.back().streamInfo.indxChunk = ch;
      if ( fcc == FCC('strf') && _Info._StreamInfos.back().streamInfo.IsVideo() ) _Info._StreamInfos.back().streamInfo.video = _Stream->Read<BITMAPINFOHEADER>();
      if ( fcc == FCC('strf') && _Info._StreamInfos.back().streamInfo.IsAudio() ) _Info._StreamInfos.back().streamInfo.audio = WaveFormatEx::FromStream( *_Stream );

      // standard index (type 2)
      if ( fcc == FCC('ix##') ) _Info._StreamInfos[fcc.StreamIndex()].indexes.push_back( Index( ch ) );

      // index entry
      if ( fcc == FCC('##db') ) _Info._StreamInfos[fcc.StreamIndex()].indexes.back().AddEntry( ch, true /*keyframe*/    );
      if ( fcc == FCC('##dc') ) _Info._StreamInfos[fcc.StreamIndex()].indexes.back().AddEntry( ch, false/*delta frame*/ );
      if ( fcc == FCC('##wb') ) _Info._StreamInfos[fcc.StreamIndex()].indexes.back().AddEntry( ch, true /*keyframe*/    );
   }

   // the captureAVI 'AVI ' and 'movi' chunk sizes are incorrect (well, only the last ones)
   virtual bool IsInsideChunk( const ChunkHeader& ch )
   {
      bool inAVIChunk  = ch.fcc == FCC('AVI ') || ch.fcc == FCC('AVIX');
      bool inMOVIChunk = ch.fcc == FCC('movi');
      if ( !inAVIChunk && !inMOVIChunk )
         return ParserBase::IsInsideChunk( ch );

      uint64_t alignBits = _Stream->Pos()&1; // chunks start on 2-byte boundaries
      if ( _Stream->Pos()+alignBits+4 >= _Stream->Size() )
         return false; // EOF

      _Stream->SetPos( _Stream->Pos() + alignBits );
      FOURCC nextFCC = _Stream->Read<FOURCC>();
      _Stream->SetPos( _Stream->Pos() - 4 - alignBits );

      if ( inAVIChunk )
         return nextFCC != FCC('RIFF'); // only a 'RIFF' or EOF ends an AVI chunk

      if ( inMOVIChunk )
         return nextFCC != FCC('idx1') /*&& nextFCC != FCC('JUNK')*/ && !ChunkHeader::IsList( nextFCC );

      assert( false );
      return false;
   }

   virtual void OnGotActualChunkHeaderSize( const ChunkHeader& ch, uint64_t actualSize )
   {
      bool needToFixSize = ch.fcc == FCC('AVI ') || ch.fcc == FCC('AVIX') || ch.fcc == FCC('movi');
      if ( !needToFixSize )
         return;
      //std::trace << FCCtoString( ch.fcc ) << " -- " << actualSize <<  " @" << ch.startPos+4 << std::endl;
      ChunkHeader fixedCh = ch;
      fixedCh.size = (uint32_t) actualSize;
      if ( ch.fcc == FCC('AVI ') ) _Info._AVIchs.push_back( fixedCh );
      if ( ch.fcc == FCC('AVIX') ) _Info._AVIchs.push_back( fixedCh );
      if ( ch.fcc == FCC('movi') ) _Info._MOVIchs.push_back( fixedCh );
   }

public:
   FillAVIIndexInfo& _Info;
};


DWORD FillAVIIndexInfo::CalcStreamLength( int streamIdx, bool firstRIFFOnly ) const
{
   if ( streamIdx < 0 || streamIdx >= (int)_StreamInfos.size() )
      return 0;

   DWORD ret = 0;

   const AVIStreamInfo& streamInfo = _StreamInfos[streamIdx];

   if ( streamInfo.streamInfo.IsVideo() )
   {
      for ( int i = 0; i < (int)streamInfo.indexes.size(); i++ )
      {
         const Index& index = streamInfo.indexes[i];
         if ( firstRIFFOnly && index.indexChunk.startPos >= _AVIchs[0].EndPos() )
            break;
         ret += (int)index.entries.size();
      }
   }

   if ( streamInfo.streamInfo.IsAudio() )
   {
      int64_t totalBytes = 0;
      for ( int i = 0; i < (int)streamInfo.indexes.size(); i++ )
      {
         const Index& index = streamInfo.indexes[i];
         if ( firstRIFFOnly && index.indexChunk.startPos >= _AVIchs[0].EndPos() )
            break;
         for ( int k = 0; k < (int)index.entries.size(); k++ )
            totalBytes += (int)index.entries[k].Size();
      }
      ret = DWORD( totalBytes / streamInfo.streamInfo.audio.BlockAlign() );
   }

   return ret;
}

void FillAVIIndexInfo::InitFrom( Stream& stream )
{
   CaptureAVIParser parser( *this );
   parser.Parse( stream );
}

void FillAVIIndexInfo::InitFrom( const std::string& filename )
{
   std::ifstream f( filename.c_str(), std::ios::in|std::ios::binary );
   assert( f );
   Stream stream( &f );
   InitFrom( stream );
}


NAMESPACE_AVI20_READ_END
