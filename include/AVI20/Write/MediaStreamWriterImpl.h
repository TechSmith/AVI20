#pragma once

#include <AVI20/Namespace.h>
#include <AVI20/AVI20Types.h>
#include <AVI20/WaveFormatEx.h>

NAMESPACE_AVI20_WRITE_BEGIN

class IMediaStreamWriterParent;
class Stream;

class MediaStreamWriterImpl
{
public:
   enum Type { VIDEO, AUDIO, INVALID };

public:
   MediaStreamWriterImpl( const BITMAPINFOHEADER& bih, int FPS, Stream* stream, int streamIndex, IMediaStreamWriterParent* parent );
   MediaStreamWriterImpl( const WaveFormatEx& wfx, Stream* stream, int streamIndex, IMediaStreamWriterParent* parent );
   bool IsVideo() const { return _Type == VIDEO; }
   bool IsAudio() const { return _Type == AUDIO; }

   void WriteSTRL();
   MediaStreamHeader VideoStreamHeader();
   MediaStreamHeader AudioStreamHeader();
   MediaStreamHeader StreamHeader();
   FOURCC fcc( bool isKeyframe ) const;
   void WriteFrame( const uint8_t* data, const uint32_t len, bool isKeyframe );
   void WriteNewStdIndex();
   void OnLeftFirstRIFF();
   int  NumEntriesInFirstRIFF() const;

private:
   Type                      _Type;
   WaveFormatEx              _Audio;
   BITMAPINFOHEADER          _Video;
   int                       _VideoFPS;
   Stream*                   _Stream; // does NOT own
   int                       _StreamIndex;
   int                       _NumEntriesLeftInStdIndex; // type 2 standard index
   int                       _NumEntriesInFirstRIFF;    // for type 1 index
   bool                      _InFirstRIFF;
   IMediaStreamWriterParent* _Parent;  // does NOT own
};

NAMESPACE_AVI20_WRITE_END