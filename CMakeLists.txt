cmake_minimum_required(VERSION 3.6)

project(
   AVI20
   VERSION 1.0
   DESCRIPTION "AVI20"
   LANGUAGES CXX C)

add_library(AVI20 STATIC
   include/AVI20/AVI20Defs.h
   include/AVI20/AVI20Types.h
   include/AVI20/Buffer.h
   include/AVI20/FCCParser.h
   include/AVI20/Namespace.h
   include/AVI20/Utl.h
   include/AVI20/WaveFormatEx.h

   include/AVI20/Read/ChunkHeader.h
   include/AVI20/Read/FillAVIIndexInfo.h
   include/AVI20/Read/FrameIndex.h
   include/AVI20/Read/IStream.h
   include/AVI20/Read/MediaStreamInfo.h
   include/AVI20/Read/MediaStreamReader.h
   include/AVI20/Read/MediaStreamReaderImpl.h
   include/AVI20/Read/ParserBase.h
   include/AVI20/Read/Reader.h
   include/AVI20/Read/Stream.h
   include/AVI20/Read/StreamPosRestorer.h
   include/AVI20/Read/WindowsStream.h

   include/AVI20/Tools/Tools.h

   include/AVI20/Write/FillOutAVIIndex.h
   include/AVI20/Write/IMediaStreamWriterParent.h
   include/AVI20/Write/MediaStreamWriter.h
   include/AVI20/Write/MediaStreamWriterImpl.h
   include/AVI20/Write/RiffStream.h
   include/AVI20/Write/StreamPosRestorerW.h
   include/AVI20/Write/StreamW.h
   include/AVI20/Write/Writer.h

   src/Buffer.cpp
   src/FCCParser.cpp
   src/Utl.cpp
   src/WaveFormatEx.cpp

   src/Read/ChunkHeader.cpp
   src/Read/FillAVIIndexInfo.cpp
   src/Read/FrameIndex.cpp
   src/Read/MediaStreamReader.cpp
   src/Read/MediaStreamReaderImpl.cpp
   src/Read/ParserBase.cpp
   src/Read/Reader.cpp
   src/Read/Stream.cpp
   src/Read/StreamPosRestorer.cpp
   src/Read/WindowsStream.cpp

   src/Write/FillOutAVIIndex.cpp
   src/Write/MediaStreamWriter.cpp
   src/Write/MediaStreamWriterImpl.cpp
   src/Write/RiffStream.cpp
   src/Write/StreamPosRestorerW.cpp
   src/Write/StreamW.cpp
   src/Write/Writer.cpp

   src/Tools/Tools.cpp
   )

target_include_directories(AVI20 PRIVATE 
   "${CMAKE_CURRENT_SOURCE_DIR}/include"
   ${STRINGUTIL_INCLUDE_DIR}
)

target_link_libraries(AVI20 PUBLIC
   StringUtil
)