#include <AVI20/FCCParser.h>
#include <AVI20/Utl.h>
#include <cassert>

NAMESPACE_AVI20_BEGIN

FCCParser::FCCParser( AVI20::FOURCC fcc )
{
   _FCC = fcc;
   _StreamIndex = -1;
   if ( ( fcc & 0x0000FFFF ) == ( FCC('ix##') & 0x0000FFFF ) ) { _FCC = FCC('ix##'); _StreamIndex = StreamIdxFromFCC( fcc ); }
   if ( ( fcc & 0xFFFF0000 ) == ( FCC('##db') & 0xFFFF0000 ) ) { _FCC = FCC('##db'); _StreamIndex = StreamIdxFromFCC( fcc ); }
   if ( ( fcc & 0xFFFF0000 ) == ( FCC('##dc') & 0xFFFF0000 ) ) { _FCC = FCC('##dc'); _StreamIndex = StreamIdxFromFCC( fcc ); }
   if ( ( fcc & 0xFFFF0000 ) == ( FCC('##wb') & 0xFFFF0000 ) ) { _FCC = FCC('##wb'); _StreamIndex = StreamIdxFromFCC( fcc ); }
}

int FCCParser::StreamIndex() const
{
   assert( _StreamIndex >= 0 );
   return _StreamIndex;
}

NAMESPACE_AVI20_END