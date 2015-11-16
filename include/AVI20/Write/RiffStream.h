#pragma once

#include <AVI20/Namespace.h>
#include <AVI20/AVI20Defs.h>
#include <AVI20/Write/StreamW.h>
#include <vector>

NAMESPACE_AVI20_BEGIN

class WaveFormatEx;

NAMESPACE_AVI20_END

NAMESPACE_AVI20_WRITE_BEGIN

class Stream;

class RiffStream
{
public:
   RiffStream( Stream& stream );

   static bool IsList( FOURCC fcc );

   void Push( FOURCC fcc );
   void Pop();
   void PushFixedSize( FOURCC fcc, DWORD size );
   void PopFixedSize();
   template<class T> void Write( FOURCC fcc, const T& t ) { PushFixedSize( fcc, sizeof(t) ); _Stream.Write( t ); PopFixedSize(); }
   void Write( FOURCC fcc, const WaveFormatEx& wfx );
   void WriteZeros( FOURCC fcc, uint64_t size );
   void WriteJUNK( int size );
   void PadTo( int blockSize, int adjust=0 );
   void WriteZeros( uint64_t size );
   int  Depth() const;

private:
   Stream&                 _Stream;
   std::vector<uint64_t>   _ChunkSizePos; // file position for chunk sizes
};

NAMESPACE_AVI20_WRITE_END