#pragma once

#include <AVI20/Namespace.h>
#include <AVI20/AVI20Defs.h>

NAMESPACE_AVI20_BEGIN

class Buffer
{
public:
   Buffer() { _Data = 0; _Size = 0; }
   Buffer( uint32_t size );
   ~Buffer();
   void Clear();
   void Alloc( uint32_t size );
   uint32_t Size() const       { return _Size; }
   uint8_t* Data()             { return _Data; }
   const uint8_t* Data() const { return _Data; }

private: // not impl -- class isn't copyable
   Buffer( const Buffer& );
   Buffer& operator=( const Buffer& );

private:
   uint8_t* _Data;
   uint32_t _Size;
};

NAMESPACE_AVI20_END