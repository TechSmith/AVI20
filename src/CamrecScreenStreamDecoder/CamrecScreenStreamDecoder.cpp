#include "stdafx.h"

#include <gtest/gtest.h>

#include <StringUtil/StringUtil.h>

#include <AVI20/Read/MediaStreamInfo.h>
#include <AVI20/Read/MediaStreamReader.h>
#include <AVI20/Read/Reader.h>
#include <AVI20/Read/WindowsStream.h>

#include <atlcomcli.h>
#include <combaseapi.h>
#include <gdiplus.h>
#include <Vfw.h>

#include <cassert>
#include <memory>

using namespace AVI20::Read;
using std::shared_ptr;
using std::unique_ptr;


namespace
{
   struct AVI20Bundle
   {
      AVI20Bundle( ::IStream *pStream )
      {
         readStream.reset( new WindowsStream( pStream ) );
         reader.reset( new Reader( *readStream ) );
         mediaStreamReader.reset( new MediaStreamReader( reader->FirstVideoStreamReader() ) );
      }

      virtual ~AVI20Bundle()
      {
         mediaStreamReader.reset();
         reader.reset();
         readStream.reset();
      }

      unique_ptr<WindowsStream>      readStream;
      unique_ptr<Reader>             reader;
      shared_ptr<MediaStreamReader>  mediaStreamReader;
   };

   BITMAPINFOHEADER bmihForCompressed( const MediaStreamInfo& msi )
   {
      BITMAPINFOHEADER bmi;

      ::memset( &bmi, 0, sizeof( BITMAPINFOHEADER ) );
      bmi.biSize = sizeof( BITMAPINFOHEADER );
      bmi.biWidth = msi.video.biWidth;
      bmi.biHeight = msi.video.biHeight;
      bmi.biPlanes = 1;
      bmi.biBitCount = msi.video.biBitCount;
      bmi.biCompression = msi.video.biCompression;
      bmi.biSizeImage = msi.header.dwSuggestedBufferSize;

      return bmi;
   }

   BITMAPINFOHEADER bmihForDecompressed( const MediaStreamInfo& msi )
   {
      BITMAPINFOHEADER bmi;

      ::memset( &bmi, 0, sizeof( BITMAPINFOHEADER ) );
      bmi.biSize = sizeof( BITMAPINFOHEADER );
      bmi.biWidth = msi.video.biWidth;
      bmi.biHeight = msi.video.biHeight;
      bmi.biPlanes = 1;
      bmi.biBitCount = 32;
      bmi.biCompression = 0;
      bmi.biSizeImage = msi.video.biWidth * msi.video.biHeight * 4;

      return bmi;
   }

   void adjustForTSCC( BITMAPINFOHEADER& bmiCompressed, BITMAPINFOHEADER& bmiDecompressed )
   {
      if ( bmiCompressed.biCompression != mmioFOURCC( 't', 's', 'c', 'c' ) )
         return;

      // TSCC seems to require 24-bit with width padded out to a multiple of 4
      bmiCompressed.biBitCount = bmiDecompressed.biBitCount = 24;
      int width = bmiCompressed.biWidth;
      while ( width % 4 )
         ++width;
      if ( width != bmiCompressed.biWidth )
         bmiCompressed.biWidth = bmiDecompressed.biWidth = width;
   }

   bool DecodeCamrecScreenStream( const std::string& path )
   {
      CComPtr<IStorage> pStorage;
      CComPtr<::IStream> pStream;
      unique_ptr<AVI20Bundle> avi20Bundle;
      HIC hic;
      HRESULT hr;

      hr = ::StgOpenStorage( StringUtil::ToCAtlString( path ), nullptr, STGM_READ | STGM_SHARE_DENY_WRITE, 0, 0, &pStorage );
      if ( FAILED( hr ) )
         return false;

      hr = pStorage->OpenStream( L"Screen_Stream.avi", nullptr, STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pStream );
      if ( FAILED( hr ) )
         return false;

      avi20Bundle.reset( new AVI20Bundle( pStream ) );

      MediaStreamInfo msi = avi20Bundle->mediaStreamReader->StreamInfo();

      hic = ::ICOpen( ICTYPE_VIDEO, msi.header.fccHandler, ICMODE_DECOMPRESS );

      BITMAPINFOHEADER bmihCompressed = bmihForCompressed( msi );
      BITMAPINFOHEADER bmihDecompresed = bmihForDecompressed( msi );

      // We don't want this for older camrecs... also seems weird that it adjusts the COMPRESSED bit-count
      if ( !msi.isTypeOneAVI )
         adjustForTSCC( bmihCompressed, bmihDecompresed );

      // We prefer 32-bit output but may need to settle for 24-bit
      LRESULT queryResult = ::ICSendMessage( hic, ICM_DECOMPRESS_QUERY, (DWORD_PTR)&bmihCompressed, (DWORD_PTR)&bmihDecompresed );
      if ( queryResult != ICERR_OK && bmihDecompresed.biBitCount != 24 )
      {
         bmihDecompresed.biBitCount = 24;
         bmihDecompresed.biSizeImage = bmihDecompresed.biWidth * bmihDecompresed.biHeight * 3;
      }

      ::ICDecompressBegin( hic, &bmihCompressed, &bmihDecompresed );

      std::unique_ptr< uint8_t[] > pUncompressed;
      pUncompressed.reset( new uint8_t[bmihDecompresed.biSizeImage] );

      std::unique_ptr< uint8_t[] > pCompressed;
      uint32_t compressedSize = 0;
      uint32_t frameCount = avi20Bundle->mediaStreamReader->NumFrames();
      uint32_t numDecoded = 0;
      for ( uint32_t i = 0; i < frameCount; ++i )
      {
         uint32_t frameSize = avi20Bundle->mediaStreamReader->FrameSize( i );
         if ( frameSize > compressedSize )
         {
            pCompressed.reset( new uint8_t[frameSize] );
            compressedSize = frameSize;
         }

         bool status = avi20Bundle->mediaStreamReader->ReadFrameData( i, pCompressed.get() );
         if ( !status )
            break;

         bmihCompressed.biSizeImage = frameSize;
         DWORD flags = avi20Bundle->mediaStreamReader->IsKeyframe( i ) ? 0 : ICDECOMPRESS_NOTKEYFRAME;
         DWORD decompressStatus = ::ICDecompress( hic, flags, &bmihCompressed, pCompressed.get(), &bmihDecompresed, pUncompressed.get() );
         if ( decompressStatus != ICERR_OK )
            break;

         ++numDecoded;
      }

      if ( hic )
      {
         ::ICDecompressEnd( hic );
         ::ICClose( hic );
      }

      avi20Bundle.reset();
      pStream.Release();
      pStorage.Release();

      return numDecoded == frameCount;
   }
}

class CamrecScreenStreamDecodingTest : public ::testing::Test
{
public:
   void SetUp() override
   {
      testMediaFolder = std::string( ABSOLUTE_PATH_TO_TEST_MEDIA_DIR );
   }

   void TearDown() override
   {

   }

   std::string testMediaFolder;
};

TEST_F( CamrecScreenStreamDecodingTest, TestCamrecWithTypeOneAVI )
{
   std::string path( testMediaFolder + "\\Customer Dictionary.camrec" );

   EXPECT_TRUE( DecodeCamrecScreenStream( path ) );
}

TEST_F( CamrecScreenStreamDecodingTest, TestCamrecWithTypeTwoAVI )
{
   std::string path( testMediaFolder + "\\1.4 part 4 remake.camrec" );

   EXPECT_TRUE( DecodeCamrecScreenStream( path ) );
}

int main( int argc, char* argv[] )
{
   HRESULT hr = ::CoInitializeEx( nullptr, COINIT_APARTMENTTHREADED );
   if ( FAILED( hr ) )
      return -1;
   ::testing::InitGoogleTest( &argc, argv );

   int result = RUN_ALL_TESTS();

   ::CoUninitialize();

   return result;
}
