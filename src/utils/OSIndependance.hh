/*
 * OSIndependance.hh
 *
 *  Created on: 25 oct. 2009
 *      Author: dloureiro
 */

#ifndef OSINDEPENDANCY_HH_
#define OSINDEPENDANCY_HH_
#ifdef WIN32
   #define SHAREDLIB __declspec(dllexport)
#else
   #define SHAREDLIB
#endif
#ifdef __WIN32__
#pragma comment(lib, "Shlwapi")
#include <string>
#ifndef WINSOCK_BOOST
#include <Winsock2.h>
#pragma message( "winsocks loaded by DIET" )
#else
#pragma message( "winsocks loaded by BOOST" )
#endif
#include <io.h>
#include <process.h>
#include <time.h>
#include "windows/stdint.h"
#include "windows/dirent.h"
#include <windows.h>
#include "Shlwapi.h"  // For PathMatchSpec
#define FNM_CASEFOLD 0
#define fnmatch(pattern, filepath, flags)  (!PathMatchSpec(filepath, pattern))

#define fsync(value) (FlushFileBuffers((HANDLE)value))

#define strdup _strdup
#define getpid _getpid
#define stricmp _stricmp
#define unlink _unlink

#include <direct.h>
#define mkdir(filepath,flags) (_mkdir(filepath))
#include <ws2tcpip.h>
#include <Iphlpapi.h>
typedef uint32_t u_int32_t;

char * getlogin(void);

SHAREDLIB int gettimeofday (struct timeval *tp, void *tz);

SHAREDLIB int gethostname(char *name, size_t namelen);

SHAREDLIB char *strtok_r(char *ptr, const char *sep, char **end);

SHAREDLIB int strcasecmp(const char *s1, const char *s2);

SHAREDLIB char *strsep( char **ppsz_string, const char *psz_delimiters );

SHAREDLIB std::string tmpDir(void);

/* getpagesize for windows */
long getpagesize (void);

#else

#include <unistd.h>
#include <sys/time.h>
#include <stdint.h>
#include <regex.h>
#include <dirent.h>
#include <sys/wait.h> // For waitpid function

/* To find a free tcp port. */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#endif

#endif /* OSINDEPENDANCY_HH_ */
