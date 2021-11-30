#ifndef _CLIBS_OS_H_
#define _CLIBS_OS_H_ 1

#if defined(__linux) || defined(__linux__) || defined(linux)
	#define OS_LINUX
#elif defined(__unix__) || defined(__unix)
	#define OS_UNIX
#elif defined(__ANDROID__)
	#define OS_ANDROID
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
	#define OS_WINDOWS

	#if defined(_WIN64)
		#define	OS_WIN64
	#else
		#define OS_WIN32
	#endif
#elif defined(__APPLE__)
	#define OS_APPLE
#endif

#endif