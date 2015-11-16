#pragma once

#include <AVI20/Namespace.h>
#include <vector>
#include <string>

NAMESPACE_AVI20_READ_BEGIN

class Stream;

NAMESPACE_AVI20_READ_END

NAMESPACE_AVI20_BEGIN

std::string       PrintRiff( const std::string& filename );
std::vector<int>  GetInterleave( Read::Stream& stream );

NAMESPACE_AVI20_END