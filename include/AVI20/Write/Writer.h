#pragma once

#include <AVI20/Namespace.h>
#include <AVI20/AVI20Defs.h>

NAMESPACE_AVI20_BEGIN

struct BITMAPINFOHEADER;
class  WaveFormatEx;

NAMESPACE_AVI20_END


NAMESPACE_AVI20_WRITE_BEGIN

class WriterImpl;
class Stream;
class MediaStreamWriter;

class Writer
{
public:
   AVI20_API Writer( Stream& stream );
   AVI20_API virtual ~Writer();

public:
   MediaStreamWriter AddMediaStream( int width, int height, int BPP, FOURCC codec, DWORD imageSize, int FPS );
   MediaStreamWriter AddMediaStream( const BITMAPINFOHEADER& bih, int FPS );
   MediaStreamWriter AddMediaStream( const WaveFormatEx& wfx );

   int NumMediaStreams() const;

public:
   void Start();
   AVI20_API void Finalize();

private:
   WriterImpl* _Impl;
};

NAMESPACE_AVI20_WRITE_END