#ifndef _CLIBS_EXPORT_H_
#define _CLIBS_EXPORT_H_ 1

#ifdef _WIN32
#define EXPORT_FUNCTION __declspec(dllexport)
#define STDCALL __stdcall
#else
#define EXPORT_FUNCTION
#define STDCALL __attribute__((__stdcall))
#endif

#endif