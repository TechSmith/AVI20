#pragma once

#include <AVI20/Namespace.h>
#include <string>

NAMESPACE_AVI20_WRITE_BEGIN

class Stream;

void FillOutAVIIndex( const std::string& filename );
void FillOutAVIIndex( Stream& stream );

NAMESPACE_AVI20_WRITE_END