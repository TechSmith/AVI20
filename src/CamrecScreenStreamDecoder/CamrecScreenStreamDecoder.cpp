#include "stdafx.h"

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
   using namespace Gdiplus;

   int GetEncoderClsid( const WCHAR* format, CLSID* pClsid )
   {
      UINT  num = 0;          // number of image encoders
      UINT  size = 0;         // size of the image encoder array in bytes
   
      ImageCodecInfo* pImageCodecInfo = NULL;
   
      GetImageEncodersSize( &num, &size );
      if ( size == 0 )
         return -1;  // Failure
   
      pImageCodecInfo = (ImageCodecInfo*)( malloc( size ) );
      if ( pImageCodecInfo == NULL )
         return -1;  // Failure
   
      GetImageEncoders( num, size, pImageCodecInfo );
   
      for ( UINT j = 0; j < num; ++j )
      {
         if ( wcscmp( pImageCodecInfo[j].MimeType, format ) == 0 )
         {
            *pClsid = pImageCodecInfo[j].Clsid;
            free( pImageCodecInfo );
            return j;  // Success
         }
      }
   
      free( pImageCodecInfo );
      return -1;  // Failure
   }

   BOOL QuickSavePic( Bitmap* pBitmap, const CString& strFilename )
   {
      CString strExt = strFilename.Mid( strFilename.ReverseFind( '.' ) + 1 );
      strExt.MakeUpper();
   
      CString strFormat;
      if ( strExt == "JPG"  )    strFormat = "image/jpeg";
      if ( strExt == "JPEG"  )   strFormat = "image/jpeg";
      if ( strExt == "GIF"  )    strFormat = "image/gif";
      if ( strExt == "PNG"  )    strFormat = "image/png";
      if ( strExt == "BMP"  )    strFormat = "image/bmp";
      CLSID clsid;
      if ( GetEncoderClsid( CStringW( strFormat ), &clsid ) == -1 )
         return FALSE;
      if ( pBitmap->Save( CStringW( strFilename ), &clsid ) != Ok )
         return FALSE;
      return TRUE;
   }
}

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
      //adjustForTSCC( bmihCompressed, bmihDecompresed );

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

         if ( i == 0 )
         {
            int stride = bmihDecompresed.biWidth * bmihDecompresed.biBitCount / 8;
            PixelFormat pf = ( bmihDecompresed.biBitCount == 24 ) ? PixelFormat24bppRGB : PixelFormat32bppRGB;
            BYTE *pLastRow = pUncompressed.get() + stride * (bmihDecompresed.biHeight - 1);
            Gdiplus::Bitmap bm( bmihDecompresed.biWidth, bmihDecompresed.biHeight, -stride, pf, pLastRow );

            CAtlString outputPath;
            TCHAR buf[MAX_PATH];
            ::GetTempPath( MAX_PATH, buf );
            outputPath.Format( L"%s\\frame_%04u.png", buf, i );
            QuickSavePic( &bm, outputPath );
         }
 
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


int main()
{
   std::string camrecWithTypeTwoAVI( "C:\\git\\CamtasiaWin3\\TSCTestMediaFiles\\1.4 part 4 remake.camrec" );
   //std::string camrecWithTypeOneAVI( "C:\\crap\\Tech Support\\Project\\OL RT 1 with math  & changes.pptx1-5.camrec" );
   std::string camrecWithTypeOneAVI( "C:\\Test Media\\camrec\\Customer Dictionary.camrec" );

   HRESULT hr = ::CoInitializeEx( nullptr, COINIT_APARTMENTTHREADED );
   if ( FAILED( hr ) )
      return -1;

   Gdiplus::GdiplusStartupInput gdiplusStartupInput;
   Gdiplus::GdiplusStartupOutput gdiplusStartupOutput;
   ULONG_PTR gdiplusToken, gdiplusBGThreadToken;
   gdiplusStartupInput.SuppressBackgroundThread = TRUE;
   Gdiplus::GdiplusStartup( &gdiplusToken,
                            &gdiplusStartupInput, &gdiplusStartupOutput );
   Gdiplus::Status stat = gdiplusStartupOutput.NotificationHook( &gdiplusBGThreadToken );

   //bool status = DecodeCamrecScreenStream( camrecWithTypeTwoAVI );
   bool status = DecodeCamrecScreenStream( camrecWithTypeOneAVI );
   assert( status == true );

   gdiplusStartupOutput.NotificationUnhook( gdiplusBGThreadToken );
   Gdiplus::GdiplusShutdown( gdiplusToken );

   ::CoUninitialize();
    return 0;
}
