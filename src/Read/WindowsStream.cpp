#include <AVI20/Read/WindowsStream.h>

#include <ObjIdl.h>

NAMESPACE_AVI20_READ_BEGIN

bool WindowsStream::IsNULL() const
{
   return _Stream == nullptr;
}

bool WindowsStream::Read( uint8_t* dest, uint64_t size )
{
   ULONG nRead = 0;
   HRESULT hr = _Stream->Read( dest, ULONG(size), &nRead );
   _CachedPos += nRead;
   return SUCCEEDED( hr ) && size == nRead;
}

uint64_t WindowsStream::Pos()
{
   return _CachedPos;
}

void WindowsStream::SetPos( uint64_t pos )
{
   LARGE_INTEGER liPos;
   ULARGE_INTEGER uliPos;
   liPos.QuadPart = int64_t( pos );
   HRESULT hr = _Stream->Seek( liPos, STREAM_SEEK_SET, &uliPos );
   if ( SUCCEEDED( hr ) )
      _CachedPos = uliPos.QuadPart;
}

void WindowsStream::SetPosToEnd()
{
   SetPos( Size() );
}

uint64_t WindowsStream::Size()
{
   if ( _CachedSize >= 0 )
      return (uint64_t)_CachedSize;

   STATSTG statStg;
   HRESULT hr = _Stream->Stat( &statStg, STATFLAG_NONAME );
   if ( SUCCEEDED( hr ) )
      _CachedSize = statStg.cbSize.QuadPart;

   return _CachedSize;
}

bool WindowsStream::IsGood() const
{
   return _Stream != nullptr;
}

void WindowsStream::Rewind()
{
   SetPos( 0ULL );
}

NAMESPACE_AVI20_READ_END