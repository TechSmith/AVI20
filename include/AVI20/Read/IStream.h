#pragma once

#include <AVI20/AVI20Defs.h>
#include <AVI20/Namespace.h>

#include <exception>

#include <stdint.h>

NAMESPACE_AVI20_BEGIN
struct AVISTDINDEX;
struct AVISTDINDEX_ENTRY;
struct AVISUPERINDEX;
struct AVISUPERINDEXENTRY;
struct BITMAPINFOHEADER;
struct MainHeader;
struct MediaStreamHeader;
struct WAVEFORMATEX;
NAMESPACE_AVI20_END

NAMESPACE_AVI20_READ_BEGIN

class IStream
{
public:
   virtual bool IsNULL() const = 0;

   //template<class T> virtual T Read() = 0;
   virtual void Read( uint32_t& d ) = 0;
   virtual void Read( AVI20::AVISTDINDEX& d ) = 0;
   virtual void Read( AVI20::AVISTDINDEX_ENTRY& d ) = 0;
   virtual void Read( AVI20::AVISUPERINDEX& d ) = 0;
   virtual void Read( AVI20::AVISUPERINDEXENTRY& d ) = 0;
   virtual void Read( AVI20::BITMAPINFOHEADER& d ) = 0;
   virtual void Read( AVI20::MainHeader& d ) = 0;
   virtual void Read( AVI20::MediaStreamHeader& d ) = 0;
   virtual void Read( AVI20::WAVEFORMATEX& d ) = 0;

   virtual bool Read( uint8_t* dest, uint64_t size ) = 0;
   virtual uint64_t Pos() = 0;
   virtual void SetPos( uint64_t pos ) = 0;
   virtual void SetPosToEnd() = 0;
   virtual uint64_t Size() = 0;
   virtual bool IsGood() const = 0;
   virtual void Rewind() = 0;

   static void ThrowException() { throw std::exception(); }
};

NAMESPACE_AVI20_READ_END