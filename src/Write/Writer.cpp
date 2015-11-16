#include <AVI20/Write/Writer.h>
#include <AVI20/Write/IMediaStreamWriterParent.h>
#include <AVI20/Write/MediaStreamWriterImpl.h>
#include <AVI20/Write/RiffStream.h>
#include <AVI20/Write/MediaStreamWriter.h>
#include <AVI20/Write/StreamW.h>
#include <AVI20/Write/FillOutAVIIndex.h>
#include <AVI20/WaveFormatEx.h>
#include <cassert>

NAMESPACE_AVI20_WRITE_BEGIN

class WriterImpl : public IMediaStreamWriterParent
{
public:
   WriterImpl( Stream& stream )
      : _Stream( &stream )
      , _Riff( *_Stream )
      , _Started( false )
      , _Finalized( false )
   {
      _RIFFStartPos.push_back( 0 );
   }
   virtual ~WriterImpl()
   {
      assert( _Started );
      assert( _Finalized );
      //if ( _Started )
      //   Finalize();
      Clear();
   }
   void Clear()
   {
      for ( int i = 0; i < (int)_MediaStreams.size(); i++ )
         delete _MediaStreams[i];
      _MediaStreams.clear();
   }

   MediaStreamWriter AddMediaStream( int width, int height, int BPP, FOURCC codec, DWORD imageSize, int FPS )
   {
      BITMAPINFOHEADER bih;
      bih.biWidth       = width;
      bih.biHeight      = height;
      bih.biPlanes      = 1;
      bih.biBitCount    = BPP;
      bih.biCompression = codec;
      bih.biSizeImage   = imageSize;
      return AddMediaStream( bih, FPS );
   }
   MediaStreamWriter AddMediaStream( const BITMAPINFOHEADER& bih, int FPS )
   {
      assert( !_Started );
      MediaStreamWriterImpl* mediaStream = new MediaStreamWriterImpl( bih, FPS, _Stream, NumMediaStreams(), this );
      _MediaStreams.push_back( mediaStream );
      return mediaStream;
   }
   MediaStreamWriter AddMediaStream( const WaveFormatEx& wfx )
   {
      assert( !_Started );
      MediaStreamWriterImpl* mediaStream = new MediaStreamWriterImpl( wfx, _Stream, NumMediaStreams(), this );
      _MediaStreams.push_back( mediaStream );
      return mediaStream;
   }
   int NumMediaStreams() const { return (int)_MediaStreams.size(); }

   int NumEntriesInFirstRIFF() const
   {
      int ret = 0;
      for ( int i = 0; i < (int)_MediaStreams.size(); i++ )
         ret += _MediaStreams[i]->NumEntriesInFirstRIFF();
      return ret;
   }

   uint64_t Idx1Size() const
   {
      return NumEntriesInFirstRIFF()*sizeof(AVIINDEXENTRY);
   }

   void WriteIdx1()
   {
      assert( NumRIFFs() == 1 );
      _Riff.WriteZeros( FCC('idx1'), Idx1Size() );
      //_Riff.WriteZeros( FCC('JUNK'), Idx1Size() );
   }

   void OnBeforeFrameWrite( int streamIndex, int frameSize )
   {
      if ( !_Started )
         Start();

      bool needIdx1 = ( NumRIFFs() == 1 );
      uint64_t currentRIFFSize = _Stream->Pos() - _RIFFStartPos.back();
      if ( currentRIFFSize + frameSize + (needIdx1?Idx1Size():0) <= 950000000ULL )
         return; // enough space

      _Riff.Pop(); // end 'movi'

      if ( needIdx1 )
         WriteIdx1();

      _Riff.Pop(); // end 'AVI ' or 'AVIX'
      StartNewRIFF();
   }

   int NumRIFFs() const { return (int)_RIFFStartPos.size(); }

   void StartNewRIFF()
   {
      _RIFFStartPos.push_back( _Stream->Pos() );
      _Riff.Push( FCC('AVIX') );
      _Riff.Push( FCC('movi') );
      for ( int i = 0; i < (int)_MediaStreams.size(); i++ )
         _MediaStreams[i]->WriteNewStdIndex(); // write 'ix00', 'ix01'...
   }

   void Start();
   void Finalize();

public:
   Stream*               _Stream;
   RiffStream            _Riff;
   std::vector<MediaStreamWriterImpl*> _MediaStreams;
   bool                                _Started;
   bool                                _Finalized;
   std::vector<uint64_t>               _RIFFStartPos;
};


void WriterImpl::Start()
{
   assert( !_Started );
   _Started = true;
   _Finalized = false;

   _Riff.Push( FCC('AVI ') );
   _Riff.Push( FCC('hdrl') );
   {
      MainHeader mainHeader;
      mainHeader.dwMicroSecPerFrame    = 0; // will be updated later
      mainHeader.dwMaxBytesPerSec      = 0;
      mainHeader.dwPaddingGranularity  = 1;
      mainHeader.dwFlags               = AVIF_ISINTERLEAVED
                                       | AVIF_WASCAPTUREFILE
                                       /*| AVIF_TRUSTCKTYPE*/
                                       | AVIF_HASINDEX;
      mainHeader.dwTotalFrames         = 0; // in first AVI section
      mainHeader.dwInitialFrames       = 0;
      mainHeader.dwStreams             = 0; // will be updated later
      mainHeader.dwSuggestedBufferSize = 0; // will be updated later (important for DShow playback)
      mainHeader.dwWidth               = 0; // will be updated later
      mainHeader.dwHeight              = 0; // will be updated later
      _Riff.Write( FCC('avih'), mainHeader );
   }

   for ( int mediaStreamIndex = 0; mediaStreamIndex < (int)_MediaStreams.size(); mediaStreamIndex++ )
      _MediaStreams[mediaStreamIndex]->WriteSTRL();

   _Riff.Push( FCC('odml') );
   _Riff.Push( FCC('dmlh') );
      _Stream->Write<uint32_t>( 0 ); // real total # frames
      _Stream->WriteZeros( 244 ); // imitate DirectShow
   _Riff.Pop();
   _Riff.Pop();

   _Riff.Pop(); // 'hdrl'
   _Riff.PadTo( 512, 12 ); // imitate DirectShow
   _Riff.Push( FCC('movi') );

   for ( int streamIdx = 0; streamIdx < (int)_MediaStreams.size(); streamIdx++ )
      _MediaStreams[streamIdx]->WriteNewStdIndex();

   assert( _Riff.Depth() == 2 );
}

void WriterImpl::Finalize()
{
   assert( _Started );

   if ( _Finalized )
      return;

   if ( NumRIFFs() == 1 )
      WriteIdx1();

   //_Riff.Pop(); // 'movi'
   //_Riff.Pop(); // 'AVI '*/

   if ( _Stream )
      FillOutAVIIndex( *_Stream );

   _Finalized = true;
}











Writer::Writer( Stream& stream )
{
   _Impl = new WriterImpl( stream );
}

Writer::~Writer()
{
   delete _Impl;
}


MediaStreamWriter Writer::AddMediaStream( int width, int height, int BPP, FOURCC codec, DWORD imageSize, int FPS ) { return _Impl->AddMediaStream( width, height, BPP, codec, imageSize, FPS ); }
MediaStreamWriter Writer::AddMediaStream( const BITMAPINFOHEADER& bih, int FPS ) { return _Impl->AddMediaStream( bih, FPS ); }
MediaStreamWriter Writer::AddMediaStream( const WaveFormatEx& wfx ) { return _Impl->AddMediaStream( wfx ); }

int Writer::NumMediaStreams() const { return _Impl->NumMediaStreams(); }

void Writer::Start() { _Impl->Start(); }
void Writer::Finalize() { _Impl->Finalize(); }



NAMESPACE_AVI20_WRITE_END
