//
// Private base definitions for the Printer Application Framework
//
// Copyright © 2019-2022 by Michael R Sweet.
//
// Licensed under Apache License v2.0.  See the file "LICENSE" for more
// information.
//

#ifndef _PAPPL_BASE_PRIVATE_H_
#  define _PAPPL_BASE_PRIVATE_H_
#  include <config.h>
#  include "base.h"
#  include <limits.h>
#  include <sys/stat.h>
#  if _WIN32
#    include <winreg.h>
#    include "win32-gettimeofday.h"
#    include "win32-pthread.h"
#    include "win32-socket.h"
#    define getuid()	0
#  else // !_WIN32
#    include <time.h>
#    include <sys/time.h>
#    include <grp.h>
#    include <poll.h>
#    include <pthread.h>
#    include <sys/fcntl.h>
#    include <sys/wait.h>
extern char **environ;
#    define O_BINARY	0		// I hate Windows...
#  endif // _WIN32


//
// The CUPS API is changed in CUPS v3...
//

#  if CUPS_VERSION_MAJOR < 3
#    define cups_len_t int
#    define cups_page_header_t cups_page_header2_t
#    define cupsArrayNew cupsArrayNew3
#    define cupsArrayGetCount (size_t)cupsArrayCount
#    define cupsArrayGetElement(a,n) cupsArrayIndex(a,(int)n)
#    define cupsArrayGetFirst cupsArrayFirst
#    define cupsArrayGetLast cupsArrayLast
#    define cupsArrayGetNext cupsArrayNext
#    define cupsArrayGetPrev cupsArrayPrev
#    define cupsGetUser cupsUser
#    define cupsRasterReadHeader cupsRasterReadHeader2
#    define cupsRasterWriteHeader cupsRasterWriteHeader2
#    define httpAddrConnect httpAddrConnect2
#    define httpConnect httpConnect2
#    define httpDecode64 httpDecode64_2
#    define httpEncode64 httpEncode64_2
#    define httpGetDateString httpGetDateString2
#    define httpRead httpRead2
#    define httpReconnect httpReconnect2
#    define httpStatusString httpStatus
#    define httpWrite httpWrite2
#    define IPP_NUM_CAST (int)
typedef cups_array_func_t cups_array_cb_t;
typedef cups_acopy_func_t cups_acopy_cb_t;
typedef cups_afree_func_t cups_afree_cb_t;
typedef cups_raster_iocb_t cups_raster_cb_t;
#    if CUPS_VERSION_MINOR < 3
#      define HTTP_STATUS_FOUND (http_status_t)302
#    endif // CUPS_VERSION_MINOR < 3
#  else
#    define cups_len_t size_t
#    define IPP_NUM_CAST (size_t)
#  endif // CUPS_VERSION_MAJOR < 3


//
// Macros...
//

#  ifdef DEBUG
#    define _PAPPL_DEBUG(...) fprintf(stderr, __VA_ARGS__)
#  else
#    define _PAPPL_DEBUG(...)
#  endif // DEBUG

#  define _PAPPL_LOC(s) s
#  define _PAPPL_LOOKUP_STRING(bit,strings) _papplLookupString(bit, sizeof(strings) / sizeof(strings[0]), strings)
#  define _PAPPL_LOOKUP_VALUE(keyword,strings) _papplLookupValue(keyword, sizeof(strings) / sizeof(strings[0]), strings)


//
// Types and structures...
//

typedef struct _pappl_ipp_filter_s	// Attribute filter
{
  cups_array_t		*ra;			// Requested attributes
  ipp_tag_t		group_tag;		// Group to copy
} _pappl_ipp_filter_t;

typedef struct _pappl_link_s		// Web interface navigation link
{
  char			*label,			// Label
			*path_or_url;		// Path or URL
  pappl_loptions_t	options;		// Link options
} _pappl_link_t;


//
// Utility functions...
//

extern ipp_t		*_papplContactExport(pappl_contact_t *contact) _PAPPL_PRIVATE;
extern void		_papplContactImport(ipp_t *col, pappl_contact_t *contact) _PAPPL_PRIVATE;
extern void		_papplCopyAttributes(ipp_t *to, ipp_t *from, cups_array_t *ra, ipp_tag_t group_tag, int quickcopy) _PAPPL_PRIVATE;
extern const char	*_papplLookupString(unsigned bit, size_t num_strings, const char * const *strings) _PAPPL_PRIVATE;
extern unsigned		_papplLookupValue(const char *keyword, size_t num_strings, const char * const *strings) _PAPPL_PRIVATE;


#endif // !_PAPPL_BASE_PRIVATE_H_
