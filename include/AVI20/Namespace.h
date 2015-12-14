#pragma once

#define NAMESPACE_AVI20_BEGIN namespace AVI20 {
#define NAMESPACE_AVI20_END   }

#define NAMESPACE_AVI20_READ_BEGIN namespace AVI20 { namespace Read {
#define NAMESPACE_AVI20_READ_END   } }

#define NAMESPACE_AVI20_WRITE_BEGIN namespace AVI20 { namespace Write {
#define NAMESPACE_AVI20_WRITE_END   } }

// Set up DLL Export/Import macro for windows
#if !defined(AVI20_API)
#if defined(_WIN32) && defined(_USRDLL)
#ifdef AVI20_EXPORTS
#define AVI20_API __declspec(dllexport)
#else
#define AVI20_API __declspec(dllimport)
#endif
#else
#define AVI20_API
#endif
#endif