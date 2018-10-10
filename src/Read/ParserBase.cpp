//#include "trace.h"
#include <fstream>
#include <stdint.h>
#include <string>
#include <vector>
#include <sstream>
#include <AVI20/AVI20Types.h>
#include <AVI20/Read/ParserBase.h>
#include <AVI20/Read/IStream.h>
#include <AVI20/Read/ChunkHeader.h>
#include <AVI20/Read/StreamPosRestorer.h>
#include <AVI20/Utl.h>

using namespace std;

NAMESPACE_AVI20_READ_BEGIN

ParserBase::ParserBase()
   : _Stream( NULL )
{
}

ParserBase::ParserBase( IStream& stream )
   : _Stream( &stream )
{
}

void ParserBase::Parse()
{
   _Stream->SetPos( 0 );
   ParseChunk( 0 );
}

void ParserBase::Parse( IStream& stream )
{
   _Stream = &stream;
   _Stream->SetPos( 0 );
   while ( _Stream->IsGood() )
      ParseChunk( 0/*current depth*/ );
   _Stream->Rewind();
}

bool ParserBase::IsInsideChunk( const ChunkHeader& ch )
{
   return _Stream->Pos() < ch.EndPos();
}

std::string ParserBase::DebugStr( int depth, const ChunkHeader& ch, bool skip )
{
   std::stringstream ss;

   ss << std::string( depth*2, ' ' );
   if ( ch.isList )
      ss << "[" << ch.Name() << "]" << " (" << ch.size << ")";
   else
      ss << ch.Name() << " (" << ch.size << ")";

   if ( skip )
      ss << " *skip*";

   return ss.str();
}

void ParserBase::ParseChunk( int depth )
{
   ChunkHeader ch = ChunkHeader::Read( *_Stream );
   if ( !_Stream->IsGood() )
      return;
   bool skip = SkipChunk( ch );

   OnGotChunkRaw( depth, ch, skip );
   //std::trace << DebugStr( depth, ch, skip ) << std::endl;

   if ( !skip )
   {
      { StreamPosRestorer fpr( *_Stream ); GotChunk( ch );   }
      { StreamPosRestorer fpr( *_Stream ); OnGotChunk( ch ); }

      if ( ch.isList )
      {
         while ( IsInsideChunk( ch ) )
            ParseChunk( depth+1 );
         OnGotActualChunkHeaderSize( ch, _Stream->Pos() - ch.StartDataPos() );
      }
   }

   if ( ch.size != 0 )
   {
      _Stream->SetPos( ch.NextPos() );
   }
   else
   {
      _Stream->SetPos( RiffRound( _Stream->Pos() ) );
   }
}

void ParserBase::GotChunk( const ChunkHeader& ch )
{
   if ( ch.fcc == FCC('avih') )
   {
      _Stream->Read( _MainHeader );
      if ( _MainHeader.dwStreams > 50 )
         IStream::ThrowException();
   }

   if ( ch.fcc == FCC('strl') )
   {
      _StreamInfo.push_back( MediaStreamInfo() );
   }

   if ( ch.fcc == FCC('strh') && ch.size == sizeof(MediaStreamHeader) && !_StreamInfo.empty() )
   {
      _Stream->Read( _StreamInfo.back().header );
   }

   if ( ch.fcc == FCC('strf') && !_StreamInfo.empty() )
   {
      if ( _StreamInfo.back().header.fccType == streamtypeVIDEO && ch.size == sizeof(BITMAPINFOHEADER) )
         _Stream->Read( _StreamInfo.back().video );
      if ( _StreamInfo.back().header.fccType == streamtypeAUDIO && ch.size >= sizeof(WAVEFORMATEX) )
         _StreamInfo.back().audio = WaveFormatEx::FromStream( *_Stream );
   }

   if ( ch.fcc == FCC('indx') && !_StreamInfo.empty() )
   {
      _StreamInfo.back().indxChunk = ch;
      //_StreamInfo.back().type2index.Read( _Stream, ch.EndPos() );
   }

   //if ( ch.fcc == FCC('movi') )
   //{
   //   _moviPos = ch.startPos+8;
   //}

   //if ( ch.fcc == FCC('idx1') )
   //{
   //   while ( _Stream.FilePos() < ch.EndPos() )
   //      _Type1IndexEntries.push_back( _Stream.Read<AVIINDEXENTRY>() );

   //   bool needToFixMovi = !_Type1IndexEntries.empty() && _Type1IndexEntries[0].dwChunkOffset < _moviPos;
   //   if ( needToFixMovi ) // untested -- but see http://www.alexander-noe.com/video/documentation/avi.pdf
   //      for ( int i = 0; i < (int) _Type1IndexEntries.size(); i++ )
   //         _Type1IndexEntries[i].dwChunkOffset += (DWORD) _moviPos;
   //}
}

MediaStreamInfo ParserBase::StreamInfo( int streamIndex ) const
{
   if ( streamIndex < 0 || streamIndex >= NumStreams() )
      return MediaStreamInfo();
   return _StreamInfo[streamIndex];
}

NAMESPACE_AVI20_READ_END
