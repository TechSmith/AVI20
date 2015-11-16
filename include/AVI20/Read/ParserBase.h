#pragma once

#include <AVI20/Namespace.h>
#include <AVI20/AVI20Types.h>
#include <AVI20/Read/MediaStreamInfo.h>
#include <vector>

NAMESPACE_AVI20_READ_BEGIN

struct ChunkHeader;
class Stream;

class ParserBase
{
public:
   ParserBase();
   ParserBase( Stream& stream );
   void Parse();
   void Parse( Stream& stream );
   void ParseChunk( int depth );

private:
   void GotChunk( const ChunkHeader& ch );

public: // mix-ins
   virtual void OnGotChunkRaw( int depth, const ChunkHeader& ch, bool skip ) {}
   virtual void OnGotChunk( const ChunkHeader& ch ) {}
   virtual bool SkipChunk( const ChunkHeader& ch ) { return false; }
   virtual bool IsInsideChunk( const ChunkHeader& ch );
   virtual void OnGotActualChunkHeaderSize( const ChunkHeader& ch, uint64_t actualSize ) {}

public:
   int             CurStreamIndex() const { return NumStreams()-1; }
   int             NumStreams() const { return (int)_StreamInfo.size(); }
   MediaStreamInfo StreamInfo( int streamIndex ) const;

protected:
   virtual std::string DebugStr( int depth, const ChunkHeader& ch, bool skip );

protected:
   Stream* _Stream;

protected:
   MainHeader                   _MainHeader;
   std::vector<MediaStreamInfo> _StreamInfo;
};

NAMESPACE_AVI20_READ_END