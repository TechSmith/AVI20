#pragma once

#include <AVI20/Namespace.h>
#include <AVI20/AVI20Defs.h>
#include <cstring> // memset

NAMESPACE_AVI20_BEGIN

#pragma pack(push, 1)

   /****************WARNING*************
   Most if not all of the structs in this file are written to disk, so the order of the properties
   is extremely important.
   ****************************************/

   struct MainHeader
   {
      MainHeader() { memset( this, 0, sizeof(*this) ); }
      DWORD dwMicroSecPerFrame;     // frame display rate (or 0L)
      DWORD dwMaxBytesPerSec;       // max. transfer rate
      DWORD dwPaddingGranularity;   // pad to multiples of this size; normally 2K.
      DWORD dwFlags;                // the ever-present flags
      DWORD dwTotalFrames;          // # frames in first movi list
      DWORD dwInitialFrames;
      DWORD dwStreams;
      DWORD dwSuggestedBufferSize;
      DWORD dwWidth;
      DWORD dwHeight;
      DWORD dwReserved[4];
   };

   struct MediaStreamHeader
   {
      MediaStreamHeader() { memset( this, 0, sizeof(*this) ); }
      bool IsVideo() { return fccType == streamtypeVIDEO; }
      bool IsAudio() { return fccType == streamtypeAUDIO; }

      FOURCC fccType;      // stream type codes
      FOURCC fccHandler;
      DWORD  dwFlags;
      //#define AVISF_DISABLED          0x00000001
      //#define AVISF_VIDEO_PALCHANGES  0x00010000
      WORD   wPriority;
      WORD   wLanguage;
      DWORD  dwInitialFrames;
      DWORD  dwScale;
      DWORD  dwRate;       // dwRate/dwScale is stream tick rate in ticks/sec
      DWORD  dwStart;
      DWORD  dwLength;
      DWORD  dwSuggestedBufferSize;
      DWORD  dwQuality;
      DWORD  dwSampleSize;
      struct
      {
         int16_t left;
         int16_t top;
         int16_t right;
         int16_t bottom;
      } rcFrame;
   };

   struct BITMAPINFOHEADER
   {
      BITMAPINFOHEADER() { memset( this, 0, sizeof(*this) ); biSize = sizeof(*this); }
      DWORD  biSize;
      LONG   biWidth;
      LONG   biHeight;
      WORD   biPlanes;
      WORD   biBitCount;
      DWORD  biCompression;
      DWORD  biSizeImage;
      LONG   biXPelsPerMeter;
      LONG   biYPelsPerMeter;
      DWORD  biClrUsed;
      DWORD  biClrImportant;
   };

   struct WAVEFORMATEX
   {
      WORD  wFormatTag;        /* format type */
      WORD  nChannels;         /* number of channels (i.e. mono, stereo...) */
      DWORD nSamplesPerSec;    /* sample rate */
      DWORD nAvgBytesPerSec;   /* for buffer estimation */
      WORD  nBlockAlign;       /* block size of data */
      WORD  wBitsPerSample;    /* Number of bits per sample of mono data */
      WORD  cbSize;            /* The count in bytes of the size of
                                 extra information (after cbSize) */
   };

   /* Flags for index */
   #define AVIIF_LIST          0x00000001L // chunk is a 'LIST'
   #define AVIIF_KEYFRAME      0x00000010L // this frame is a key frame.
   #define AVIIF_FIRSTPART     0x00000020L // this frame is the start of a partial frame.
   #define AVIIF_LASTPART      0x00000040L // this frame is the end of a partial frame.
   #define AVIIF_MIDPART       (AVIIF_LASTPART|AVIIF_FIRSTPART)

   #define AVIIF_NOTIME	       0x00000100L // this frame doesn't take any time
   #define AVIIF_COMPUSE       0x0FFF0000L // these bits are for compressor use

   struct AVIINDEXENTRY
   {
      DWORD ckid;
      DWORD dwFlags;
      DWORD dwChunkOffset;
      DWORD dwChunkLength;
   };

   struct AVISUPERINDEXENTRY
   {
      DWORDLONG   qwOffset;     // 64 bit offset to sub index chunk
      DWORD       dwSize;       // 32 bit size of sub index chunk
      DWORD       dwDuration;   // time span of subindex chunk (in stream ticks)
   };

   struct AVISUPERINDEX
   {
      WORD     wLongsPerEntry;    // ==4
      BYTE     bIndexSubType;     // ==0 (frame index) or AVI_INDEX_SUB_2FIELD
      BYTE     bIndexType;        // ==AVI_INDEX_OF_INDEXES
      DWORD    nEntriesInUse;     // offset of next unused entry in aIndex
      DWORD    dwChunkId;         // chunk ID of chunks being indexed, (i.e. RGB8)
      DWORD    dwReserved[3];     // must be 0
   };

   #define AVISTDINDEX_DELTAFRAME ( 0x80000000) // Delta frames have the high bit set
   #define AVISTDINDEX_SIZEMASK   (~0x80000000)

   struct AVISTDINDEX_ENTRY
   {
   public:
      AVISTDINDEX_ENTRY() {}
      AVISTDINDEX_ENTRY( DWORD dwOffset, DWORD dwSize ) : dwOffset(dwOffset), dwSize(dwSize) {}
      AVISTDINDEX_ENTRY( DWORD dwOffset, DWORD dwSize, bool isKeyframe ) : dwOffset(dwOffset), dwSize(dwSize | (isKeyframe?0:AVISTDINDEX_DELTAFRAME)) {}
      DWORD Offset() const       { return dwOffset; }
      DWORD Size() const         { return dwSize & AVISTDINDEX_SIZEMASK; }
      DWORD RawSize() const      { return dwSize; }
      bool  IsKeyframe() const   { return ( dwSize & AVISTDINDEX_DELTAFRAME ) == 0; }
   private:
      DWORD dwOffset;       // 32 bit offset to data (points to data, not riff header)
      DWORD dwSize;         // 31 bit size of data (does not include size of riff header), bit 31 is deltaframe bit
   };

   struct AVISTDINDEX
   {
      WORD        wLongsPerEntry;    // ==2
      BYTE        bIndexSubType;     // ==0
      BYTE        bIndexType;        // ==AVI_INDEX_OF_CHUNKS
      DWORD       nEntriesInUse;     // offset of next unused entry in aIndex
      DWORD       dwChunkId;         // chunk ID of chunks being indexed, (i.e. RGB8)
      DWORDLONG   qwBaseOffset;      // base offset that all index intries are relative to
      DWORD       dwReserved_3;      // must be 0
   };

   struct RIFFCHUNK
   {
      RIFFCHUNK() {}
      RIFFCHUNK( FOURCC fcc, DWORD cb ) : fcc(fcc), cb(cb) {}
      FOURCC fcc;
      DWORD  cb;
   };

   struct RIFFLIST
   {
      RIFFLIST() {}
      RIFFLIST( FOURCC fccListType, DWORD cb ) : fcc(FCC('LIST')),  cb(cb), fccListType(fccListType) {}
      static RIFFLIST AVI()  { RIFFLIST r( FCC('AVI '), 0 ); r.fcc = FCC('RIFF'); return r; }
      static RIFFLIST AVIX() { RIFFLIST r( FCC('AVIX'), 0 ); r.fcc = FCC('RIFF'); return r; }
      FOURCC fcc;
      DWORD  cb;
      FOURCC fccListType;
   };

#pragma pack(pop)



NAMESPACE_AVI20_END
