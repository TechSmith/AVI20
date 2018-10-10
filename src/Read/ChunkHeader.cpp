#include <AVI20/Read/ChunkHeader.h>
#include <AVI20/Read/IStream.h>
#include <AVI20/Utl.h>
#include <cstring> // memset

NAMESPACE_AVI20_READ_BEGIN

std::string ChunkHeader::Name() const { return FCCtoString( fcc ); }

ChunkHeader ChunkHeader::Read( IStream& stream )
{
   ChunkHeader ret;
   ret.startPos = stream.Pos();
   //ret.fcc      = stream.Read<uint32_t>();
   //ret.size     = stream.Read<uint32_t>();
   stream.Read( ret.fcc );
   stream.Read( ret.size );
   ret.isList   = ChunkHeader::IsList( ret.fcc );
   if ( ret.isList )
      //ret.fcc = stream.Read<uint32_t>();
      stream.Read( ret.fcc );
   return ret;
}

ChunkHeader ChunkHeader::Invalid()
{
   ChunkHeader ret;
   memset( &ret, 0, sizeof(ret) );
   return ret;
}

NAMESPACE_AVI20_READ_END