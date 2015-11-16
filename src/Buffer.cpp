#include <AVI20/Buffer.h>

NAMESPACE_AVI20_BEGIN

Buffer::Buffer( uint32_t size )
{
   _Data = 0;
   _Size = 0;
   Alloc( size );
}

Buffer::~Buffer()
{
   Clear();
}

void Buffer::Alloc( uint32_t size )
{
   Clear();
   _Size = size;
   _Data = new uint8_t[_Size];
}

void Buffer::Clear()
{
   delete [] _Data;
   _Data = 0;
   _Size = 0;
}

NAMESPACE_AVI20_END