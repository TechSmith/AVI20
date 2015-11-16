#pragma once

#include <AVI20/Namespace.h>
#include <cstring> // memset
#include <AVI20/AVI20Types.h>
#include <AVI20/Read/ChunkHeader.h>
#include <AVI20/WaveFormatEx.h>

NAMESPACE_AVI20_READ_BEGIN

struct MediaStreamInfo
{
   MediaStreamInfo() { memset( this, 0, sizeof(*this) ); }
   bool IsValid() const { return header.fccType != 0; }
   bool IsVideo() const { return header.fccType == streamtypeVIDEO; }
   bool IsAudio() const { return header.fccType == streamtypeAUDIO; }
   double VideoFPS() const { return IsVideo() ? (double)header.dwRate/header.dwScale : 0; }
   MediaStreamHeader header;
   BITMAPINFOHEADER  video;
   WaveFormatEx      audio;
   ChunkHeader       indxChunk;
};

NAMESPACE_AVI20_READ_END