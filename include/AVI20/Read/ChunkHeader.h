#pragma once

#include <AVI20/Namespace.h>
#include <AVI20/AVI20Defs.h>
#include <string>
#include <cstring> // memset

NAMESPACE_AVI20_READ_BEGIN

class Stream;

struct ChunkHeader
{
   ChunkHeader() { memset( this, 0, sizeof(*this) ); }
   uint32_t fcc;
   uint32_t size;
   bool     isList;
   uint64_t startPos;

   std::string Name() const;
   uint64_t PadBytes() const { return size&1; }
   uint64_t StartDataPos() const { return startPos + 8; }
   uint64_t EndPos() const { return startPos + 8 + size; }
   uint64_t NextPos() const { return EndPos() + PadBytes(); }
   bool IsValid() const { return fcc != 0; }

   static bool IsList( uint32_t fcc ) { return fcc == FCC('RIFF') || fcc == FCC('LIST'); }
   static ChunkHeader Read( Stream& stream );
   static ChunkHeader Invalid();
};

NAMESPACE_AVI20_READ_END