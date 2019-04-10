#include <fstream>
#include <stdint.h>
#include <limits>
#include <string>
#include <vector>
#include <exception>
#include <algorithm>
#include <AVI20/AVI20Types.h>
#include <AVI20/Read/Reader.h>
#include <AVI20/Read/ParserBase.h>
#include <AVI20/Read/ChunkHeader.h>
#include <AVI20/Read/MediaStreamReader.h>
#include <AVI20/Read/MediaStreamReaderImpl.h>

using namespace std;

NAMESPACE_AVI20_READ_BEGIN

class BasicInfoParser : public ParserBase
{
public:
   BasicInfoParser( IStream& stream )
      : ParserBase( stream )
      , _MoviPos( 0ULL )
   {

   }

   bool SkipChunk( const ChunkHeader& ch )
   {
      if ( ch.fcc == FCC( 'movi' ) )
      {
         _MoviPos = ch.StartDataPos();
         return true;
      }
      return false;
   }

   uint64_t MoviPos() const { return _MoviPos;  }

protected:
   uint64_t _MoviPos;
};

class ReaderImpl
{
public:
   ReaderImpl( IStream& stream )
      : _Stream( stream )
      , _Parser( stream )
   {
      _Parser.Parse();
   }

   virtual ~ReaderImpl()
   {
      Clear();
   }

   void Clear()
   {
      for ( int i = 0; i < (int)_StreamReader.size(); i++ )
         delete _StreamReader[i];
      _StreamReader.clear();
   }

   int NumStreams() const
   {
      return _Parser.NumStreams();
   }

   MediaStreamInfo StreamInfo( int streamIndex ) const
   {
      return _Parser.StreamInfo( streamIndex );
   }

   MediaStreamReader StreamReader( int streamIndex )
   {
      if ( streamIndex > _Parser.NumStreams() )
         return MediaStreamReader( NULL );
      Realize( streamIndex );
      return _StreamReader[streamIndex];
   }

   MediaStreamReader FirstVideoStreamReader()
   {
      for ( int i = 0; i < _Parser.NumStreams(); i++ )
      {
         if ( !_Parser.StreamInfo( i ).header.IsVideo() )
            continue;

         return StreamReader( i );
      }
      return MediaStreamReader( NULL );
   }

   MediaStreamReader FirstAudioStreamReader()
   {
      for ( int i = 0; i < _Parser.NumStreams(); i++ )
      {
         if ( !_Parser.StreamInfo( i ).header.IsAudio() )
            continue;

         return StreamReader( i );
      }
      return MediaStreamReader( NULL );
   }

   void Realize( int streamIndex )
   {
      if ( streamIndex >= (int)_StreamReader.size() )
         _StreamReader.resize( streamIndex+1 );
      if ( _StreamReader[streamIndex] )
         return;
      _StreamReader[streamIndex] = new MediaStreamReaderImpl( _Stream, _Parser.StreamInfo( streamIndex ) );
   }

   double Duration() const
   {
      double ret = 0;
      for ( int i = 0; i < (int) _StreamReader.size(); i++ )
         ret = std::max( ret, _StreamReader[i]->Duration() );
      return ret;
   }

private:
   IStream&                             _Stream;
   std::vector<MediaStreamReaderImpl*> _StreamReader;
   BasicInfoParser                     _Parser;
};


Reader::Reader()
{
   _Impl = NULL;
}

Reader::~Reader()
{
   delete _Impl;
}

Reader::Reader( IStream& stream )
{
   _Impl = new ReaderImpl( stream );
}

int                      Reader::NumStreams() const                  { return _Impl->NumStreams(); }
Read::MediaStreamInfo    Reader::StreamInfo( int streamIndex ) const { return _Impl->StreamInfo( streamIndex ); }
Read::MediaStreamReader  Reader::StreamReader( int streamIndex )     { return _Impl->StreamReader( streamIndex ); }
Read::MediaStreamReader  Reader::FirstVideoStreamReader()            { return _Impl->FirstVideoStreamReader(); }
Read::MediaStreamReader  Reader::FirstAudioStreamReader()            { return _Impl->FirstAudioStreamReader(); }
double                   Reader::Duration() const                    { return _Impl->Duration(); }

NAMESPACE_AVI20_READ_END
