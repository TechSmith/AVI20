# AVI20
Library for reading and writing AVI 2.0 files (which supports files larger than 4GB)

Sample usage:

```
#include <AVI20/Write/Writer.h>
#include <AVI20/Write/StreamW.h>
#include <AVI20/Write/MediaStreamWriter.h>
#include <AVI20/WaveFormatEx.h>
#include <fstream>
#include <vector>

using namespace AVI20;
using namespace AVI20::Write;

const int WIDTH      = 320;
const int HEIGHT     = 240;
const int BPP        = 32; // bits per pixel
const int FPS        = 30; // frames per second
const int FRAME_SIZE = WIDTH * HEIGHT * 4;

const double PI = acos(0.)*2;

void drawTunnelEffect( int frameIndex, uint8_t* pixelBuffer )
{
   double time = (double) frameIndex/FPS;
   for ( int y = 0; y < HEIGHT; y++ )
   for ( int x = 0; x < WIDTH; x++ )
   {
      double u = double(2*x-WIDTH ) / HEIGHT;
      double v = double(2*y-HEIGHT) / HEIGHT;
      double len = sqrt( u*u + v*v );
      double angle = atan2( v, u ) + .1 * time;
      uint8_t color = len < .2 ? 0 : uint8_t( lround( angle/PI * 256 ) ^ lround( (1/len + time) * 256 ) );
      *pixelBuffer++ = color;
      *pixelBuffer++ = color;
      *pixelBuffer++ = color;
      *pixelBuffer++ = 0xFF;
   }
}

int main()
{   
   std::fstream fileStream( "out.avi", std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary );

   Stream stream( &fileStream );
   Writer writer( stream );

   MediaStreamWriter videoStreamWriter = writer.AddMediaStream( WIDTH, HEIGHT, BPP, 0/*uncompressed codec*/, FRAME_SIZE, 30 );   
   //MediaStreamWriter audioStreamWriter = writer.AddMediaStream( WaveFormatEx::PCM( 44100 /*sample rate*/, 16 /*bit depth*/, 1 /*# channels*/ ) );

   writer.Start();

   // write the video
   {
      std::vector<uint8_t> pixelBuffer( FRAME_SIZE );
      for ( int frameIndex = 0; frameIndex < 5*30; frameIndex++ ) 
      {
         drawTunnelEffect( frameIndex, pixelBuffer.data() );
         videoStreamWriter.WriteFrame( pixelBuffer.data(), FRAME_SIZE, true/*keyframe*/ );
      }
   }

   //// write the audio
   //{
   //   std::vector<int16_t> audioBuffer( 44100*5 );
   //   for ( int i = 0; i < (int) audioBuffer.size(); i++ ) 
   //      audioBuffer[i] = sin( i*200./*frequency*//44100*PI*2 ) * 1000. /*volume*/;
   //   audioStreamWriter.WriteFrame( audioBuffer.data(), audioBuffer.size()*sizeof(int16_t), true /*keyframe*/ );
   //}

   writer.Finalize();

   return 0;
}
```
