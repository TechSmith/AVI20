#pragma once

#include <AVI20/Namespace.h>

NAMESPACE_AVI20_READ_BEGIN

struct MediaStreamInfo;
class ReaderImpl;
class MediaStreamReader;
class Stream;

class Reader
{
public:
   Reader();
   Reader( Stream& stream );
   virtual ~Reader();

   int               NumStreams() const;
   MediaStreamInfo   StreamInfo( int streamIndex ) const;
   MediaStreamReader StreamReader( int streamIndex );
   MediaStreamReader FirstVideoStreamReader();
   MediaStreamReader FirstAudioStreamReader();
   double            Duration() const;

private:
   ReaderImpl* _Impl;
};

NAMESPACE_AVI20_READ_END