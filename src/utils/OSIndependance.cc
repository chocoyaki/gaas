/*
 * OSIndependance.cc
 *
 *  Created on: 25 oct. 2009
 *      Author: dloureiro
 */


#include <time.h>
#include <string>
#ifdef __WIN32__
#include <sys/timeb.h>
#endif
#include "OSIndependance.hh"

using namespace std;

char* getlogin(void)
{
	//char* result = new char[256]; 
	//GetUserName(result,(LPDWORD)256); 
	return getenv("USERNAME");//result;

}

int gettimeofday (struct timeval *tp, void *tz)
{
	struct _timeb timebuffer;
	_ftime (&timebuffer);
	tp->tv_sec = timebuffer.time;
	tp->tv_usec = timebuffer.millitm * 1000;
	return 0;
}

int gethostname(char *name, size_t namelen){

// Add ws2_32.lib to your linker options

  WSADATA WSAData;

// Initialize winsock dll
  if(::WSAStartup(MAKEWORD(1, 0), &WSAData) == FALSE)
  // Error handling

if(::gethostname(name, namelen - 1)){
  return -1; // must be something well defined
}
  // Error -> call 'WSAGetLastError()'

// Cleanup
return WSACleanup();

}

static char *strtok_pos;
 
char *
strtok_r(char *ptr, const char *sep, char **end)
{
    if (!ptr)
        /* we got NULL input so then we get our last position instead */
        ptr = *end;
 
    /* pass all letters that are including in the separator string */
    while (*ptr && strchr(sep, *ptr))
        ++ptr;
 
    if (*ptr) {
        /* so this is where the next piece of string starts */
        char *start = ptr;
 
        /* set the end pointer to the first byte after the start */
        *end = start + 1;
 
        /* scan through the string to find where it ends, it ends on a
           null byte or a character that exists in the separator string */
        while (**end && !strchr(sep, **end))
            ++*end;
 
        if (**end) {
            /* the end is not a null byte */
            **end = '\0';  /* zero terminate it! */
            ++*end;        /* advance last pointer to beyond the null byte */
        }
 
        return start; /* return the position where the string starts */
    }
 
    /* we ended up on a null byte, there are no more strings to find! */
    return NULL;
}
 
int strcasecmp(const char *s1, const char *s2){
  return stricmp(s1,s2);
}


char *strsep( char **ppsz_string, const char *psz_delimiters )
{
    char *psz_string = *ppsz_string;
    if( !psz_string )
        return NULL;

    char *p = strpbrk( psz_string, psz_delimiters );
    if( !p )
    {
        *ppsz_string = NULL;
        return psz_string;
    }
    *p++ = '\0';

    *ppsz_string = p;
    return psz_string;
}

std::string tmpDir(){
#ifdef __WIN32__

	std::string temp = std::string(std::getenv("TEMP"))+std::string("\\");
	char* longPath = (char*)malloc(MAX_PATH*sizeof(char));
	GetLongPathName(temp.c_str(),longPath,MAX_PATH);
	std::string result = std::string(longPath);
	free(longPath);
	return result;

#else
	return "/tmp/";
#endif
}

/* getpagesize for windows */
long getpagesize (void) {
    static long g_pagesize = 0;
    if (! g_pagesize) {
        SYSTEM_INFO system_info;
        GetSystemInfo (&system_info);
        g_pagesize = system_info.dwPageSize;
    }
    return g_pagesize;
}