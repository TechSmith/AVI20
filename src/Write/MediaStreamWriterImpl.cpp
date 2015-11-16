#include <AVI20/Write/MediaStreamWriterImpl.h>
#include <AVI20/Write/StreamW.h>
#include <AVI20/Write/IMediaStreamWriterParent.h>
#include <AVI20/Write/RiffStream.h>
#include <AVI20/AVI20Types.h>
#include <AVI20/Utl.h>
#include <cassert>

NAMESPACE_AVI20_WRITE_BEGIN


MediaStreamWriterImpl::MediaStreamWriterImpl( const BITMAPINFOHEADER& bih, int FPS, Stream* stream, int streamIndex, IMediaStreamWriterParent* parent )
   : _Type(VIDEO)
   , _Video(bih)
   , _VideoFPS(FPS)
   , _Stream(stream)
   , _StreamIndex(streamIndex)
   , _NumEntriesLeftInStdIndex(0)
   , _NumEntriesInFirstRIFF(0)
   , _InFirstRIFF(true)
   , _Parent(parent) {}

MediaStreamWriterImpl::MediaStreamWriterImpl( const WaveFormatEx& wfx, Stream* stream, int streamIndex, IMediaStreamWriterParent* parent )
   : _Type(AUDIO)
   , _Audio(wfx)
   , _Stream(stream)
   , _StreamIndex(streamIndex)
   , _NumEntriesLeftInStdIndex(0)
   , _NumEntriesInFirstRIFF(0)
   , _InFirstRIFF(true)
   , _Parent(parent)
{
}


MediaStreamHeader MediaStreamWriterImpl::VideoStreamHeader()
{
   MediaStreamHeader hdr;
   hdr.fccType    = streamtypeVIDEO;
   hdr.fccHandler = _Video.biCompression;
   hdr.dwScale    = 1;
   hdr.dwRate     = _VideoFPS;
   hdr.dwLength   = 0; // in first AVI section
   hdr.dwSuggestedBufferSize = 0;
   return hdr;
}

MediaStreamHeader MediaStreamWriterImpl::AudioStreamHeader()
{
   MediaStreamHeader hdr;
   hdr.fccType    = streamtypeAUDIO;
   hdr.fccHandler = 0;
   hdr.dwSampleSize  = _Audio.BlockAlign();
   hdr.dwScale       = _Audio.BlockAlign();
   hdr.dwRate        = _Audio.IsPCM() ? _Audio.SamplesPerSec() * _Audio.BlockAlign() : _Audio.AvgBytesPerSec();
   hdr.dwLength   = 0; // in first AVI section
   hdr.dwSuggestedBufferSize = 0;
   return hdr;
}

MediaStreamHeader MediaStreamWriterImpl::StreamHeader()
{
   return IsVideo() ? VideoStreamHeader() : AudioStreamHeader();
}

FOURCC MediaStreamWriterImpl::fcc( bool isKeyframe ) const
{
   if ( IsVideo() ) return isKeyframe ? FCC2('##db',_StreamIndex) : FCC2('##dc',_StreamIndex);
   if ( IsAudio() ) return FCC2('##wb',_StreamIndex);
   assert( false );
   return 0;
}

void MediaStreamWriterImpl::WriteFrame( const uint8_t* data, const uint32_t len, bool isKeyframe )
{
   _Parent->OnBeforeFrameWrite( _StreamIndex, len );
   if ( _NumEntriesLeftInStdIndex <= 0 )
      WriteNewStdIndex();

   RiffStream riff( *_Stream );
   riff.PushFixedSize( fcc( isKeyframe ), len );
   _Stream->Write( data, len );
   riff.PopFixedSize();
   riff.PadTo( 512 ); // imitate DirectShow
   _NumEntriesLeftInStdIndex--;
   if ( _InFirstRIFF )
      _NumEntriesInFirstRIFF++;
}

void MediaStreamWriterImpl::WriteNewStdIndex()
{
   const int STDINDEX_SIZE_IN_BYTES = 32248;
   RiffStream riff( *_Stream );
   riff.WriteZeros( FCC2('ix##', _StreamIndex ), STDINDEX_SIZE_IN_BYTES );
   _NumEntriesLeftInStdIndex = ( STDINDEX_SIZE_IN_BYTES - sizeof(AVISTDINDEX) ) / sizeof(AVISTDINDEX_ENTRY);
}

void MediaStreamWriterImpl::OnLeftFirstRIFF()
{
   _InFirstRIFF = false;
}

int  MediaStreamWriterImpl::NumEntriesInFirstRIFF() const
{
   return _NumEntriesInFirstRIFF;
}


void MediaStreamWriterImpl::WriteSTRL()
{
   const int SUPERINDEX_SIZE_IN_BYTES = 32248; // this is the value DirectShow uses

   RiffStream riff( *_Stream );
   riff.Push( FCC('strl') );
   {
      riff.Write( FCC('strh'), StreamHeader() );
      if ( IsVideo() ) riff.Write( FCC('strf'), _Video );
      if ( IsAudio() ) riff.Write( FCC('strf'), _Audio );
      riff.WriteZeros( FCC('indx'), SUPERINDEX_SIZE_IN_BYTES );
   }
   riff.Pop(); // 'strl'
}

NAMESPACE_AVI20_WRITE_END
