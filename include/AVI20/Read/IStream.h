#pragma once

#include <AVI20/AVI20Defs.h>
#include <AVI20/Namespace.h>

#include <exception>

#include <stdint.h>

NAMESPACE_AVI20_READ_BEGIN

class IStream
{
public:
   virtual bool IsNULL() const = 0;

   template<typename T> void Read( T& t ) { Read( (uint8_t *)&t, sizeof( T ) ); }
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