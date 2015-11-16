#include <AVI20/Tools/Tools.h>
#include <AVI20/Read/ParserBase.h>
#include <AVI20/Read/Stream.h>
#include <sstream>
#include <fstream>

NAMESPACE_AVI20_BEGIN

class PrintRIFF : public Read::ParserBase
{
public:
   virtual bool SkipChunk( const Read::ChunkHeader& ch ) { return ch.fcc == FCC('movi'); }
   virtual void OnGotChunkRaw( int depth, const Read::ChunkHeader& ch, bool skip )
   {
      ss << DebugStr( depth, ch, skip ) << std::endl;
   }

public:
   std::stringstream ss;
};

std::string PrintRiff( const std::string& filename )
{
   std::ifstream f( filename.c_str(), std::ios::in|std::ios::binary );
   Read::Stream stream( &f );
   PrintRIFF pr;
   pr.Parse( stream );
   return pr.ss.str();
}



//class CheckIndexesParse : public Read::ParserBase
//{
//public:
//   virtual void OnGotChunk( const Read::ChunkHeader& ch )
//   {
//      if ( ch.fcc == FCC('indx') )
//         OnGot( _Stream->Read<AVISUPERINDEX>() );
//   }
//   virtual void OnGot( const AVISUPERINDEX& superIndex )
//   {
//      avi20::trace << "superIndex" << std::endl;
//      avi20::trace << " ~dwChunkId  = " << FCCtoString( superIndex.dwChunkId ) << std::endl;
//      avi20::trace << " ~#entries   = " << superIndex.nEntriesInUse << std::endl;
//
//      for ( int i = 0; i < (int)superIndex.nEntriesInUse; i++ )
//         OnGot( _Stream->Read<AVISUPERINDEXENTRY>() );
//   }
//   virtual void OnGot( const AVISUPERINDEXENTRY& sie )
//   {
//      avi20::trace << "   -duration = " << sie.dwDuration << std::endl;
//      avi20::trace << "   -offset   = " << sie.qwOffset << std::endl;
//      avi20::trace << "   -size     = " << sie.dwSize << std::endl;
//      Read::StreamPosRestorer restorer( *_Stream );
//      _Stream->SetPos( sie.qwOffset );
//      RIFFCHUNK rc = _Stream->Read<RIFFCHUNK>();
//      OnGot( rc, _Stream->Read<AVISTDINDEX>() );
//   }
//   virtual void OnGot( const RIFFCHUNK& ch, const AVISTDINDEX& idx )
//   {
//      avi20::trace << "     +fcc          = " << FCCtoString( ch.fcc ) << std::endl;
//      avi20::trace << "     +chunk id     = " << FCCtoString( idx.dwChunkId ) << std::endl;
//      avi20::trace << "     +#entries     = " << idx.nEntriesInUse << std::endl;
//      avi20::trace << "     +qwBaseOffset = " << idx.qwBaseOffset << std::endl;
//      for ( int i = 0; i < (int) idx.nEntriesInUse; i++ )
//         OnGot( idx, _Stream->Read<AVISTDINDEX_ENTRY>() );
//   }
//   virtual void OnGot( const AVISTDINDEX& idx, const AVISTDINDEX_ENTRY& entry )
//   {
//      uint64_t pos = idx.qwBaseOffset + entry.Offset();
//
//      avi20::trace << "       `pos=" << pos << "\tsize=" << entry.Size() << "\t" << (entry.IsKeyframe()?"KEY":"") << std::endl;
//   }
//};


class GetInterleaveParser : public Read::ParserBase
{
public:
   virtual void OnGotChunk( const Read::ChunkHeader& ch )
   {
      if ( ch.fcc == FCC('01wb') ) isVideo.push_back( 0 );
      if ( ch.fcc == FCC('00db') ) isVideo.push_back( 1 );
      if ( ch.fcc == FCC('00dc') ) isVideo.push_back( 1 );
   }

public:
   std::vector<int> isVideo;
};

std::vector<int> GetInterleave( Read::Stream& stream )
{
   GetInterleaveParser gip;
   gip.Parse( stream );
   return gip.isVideo;
}

NAMESPACE_AVI20_END
