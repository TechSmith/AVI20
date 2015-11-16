#include <AVI20/Write/RiffStream.h>
#include <AVI20/Write/StreamW.h>
#include <AVI20/AVI20Types.h>
#include <AVI20/WaveFormatEx.h>
#include <algorithm>
#include <cassert>

NAMESPACE_AVI20_WRITE_BEGIN

RiffStream::RiffStream( Stream& stream )
   : _Stream( stream )
{
}

bool RiffStream::IsList( FOURCC fcc )
{
   return fcc == FCC('AVI ')
       || fcc == FCC('AVIX')
       || fcc == FCC('hdrl')
       || fcc == FCC('strl')
       || fcc == FCC('odml')
       || fcc == FCC('movi');
}

void RiffStream::Push( FOURCC fcc )
{
   uint64_t pos = _Stream.Pos();
   _ChunkSizePos.push_back( _Stream.Pos()+4 );
   if ( IsList(fcc) )
      _Stream.Write( fcc == FCC('AVI ') ? RIFFLIST::AVI()
                   : fcc == FCC('AVIX') ? RIFFLIST::AVIX()
                   : RIFFLIST( fcc, 0 ) );
   else
      _Stream.Write( RIFFCHUNK( fcc, 0 ) );
}

void RiffStream::Pop()
{
   _Stream.WriteAt( uint32_t( _Stream.Pos() - _ChunkSizePos.back() - 4 ), _ChunkSizePos.back() );
   _ChunkSizePos.pop_back();
   if ( _Stream.Pos()&1 )
      _Stream.WriteZeros( 1 );
}

void RiffStream::PushFixedSize( FOURCC fcc, DWORD size )
{
   _ChunkSizePos.push_back( 0 );
   assert( !IsList( fcc ) );
   if ( IsList( fcc ) )
      return;

   _Stream.Write( RIFFCHUNK( fcc, size ) );
}

void RiffStream::PopFixedSize()
{
   assert( _ChunkSizePos.back() == 0 );
   _ChunkSizePos.pop_back();
   if ( _Stream.Pos()&1 )
      _Stream.WriteZeros( 1 );
}

void RiffStream::WriteZeros( FOURCC fcc, uint64_t size )
{
   PushFixedSize( fcc, (DWORD)size );
   WriteZeros( size );
   PopFixedSize();
}

void RiffStream::WriteJUNK( int size )
{
   PushFixedSize( FCC('JUNK'), size );
   _Stream.WriteZeros( size );
   PopFixedSize();
}

void RiffStream::PadTo( int blockSize, int adjust/*=0*/ )
{
   int excess = _Stream.Pos() % blockSize;
   if ( excess == 0 )
      return;
   int junkSize = blockSize-8-excess-adjust;
   while ( junkSize < 0 )
      junkSize += blockSize;
   WriteJUNK( junkSize );
}

void RiffStream::WriteZeros( uint64_t size )
{
   _Stream.WriteZeros( size );
}

int RiffStream::Depth() const
{
   return (int)_ChunkSizePos.size();
}

void RiffStream::Write( FOURCC fcc, const WaveFormatEx& wfx )
{
   PushFixedSize( fcc, wfx.TotalSize() );
   wfx.WriteToStream( _Stream );
   PopFixedSize();
}

NAMESPACE_AVI20_WRITE_END
