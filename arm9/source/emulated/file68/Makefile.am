mysources=\
 alloc68.c\
 debugmsg68.c\
 endian68.c\
 error68.c\
 file68.c\
 gzip68.c\
 init68.c\
 ice68.c\
 istream68.c\
 istream68_curl.c\
 istream68_fd.c\
 istream68_file.c\
 istream68_mem.c\
 istream68_null.c\
 istream68_z.c\
 registry68.c\
 rsc68.c\
 string68.c\
 url68.c

myheaders=\
 alloc68.h\
 debugmsg68.h\
 endian68.h\
 error68.h\
 file68.h\
 gzip68.h\
 ice68.h\
 init68.h\
 istream68.h\
 istream68_def.h\
 istream68_curl.h\
 istream68_fd.h\
 istream68_file.h\
 istream68_mem.h\
 istream68_null.h\
 istream68_z.h\
 registry68.h\
 rsc68.h\
 string68.h\
 url68.h

lib_LTLIBRARIES = libfile68.la
libfile68_la_SOURCES = $(mysources) $(myheaders)
libfile68_la_LDFLAGS = -version-info 2:2:0 -release @VERSION@ @CURL_LIB@ @Z_LIB@
libfile68_la_CFLAGS = -Wno-multichar -DSC68_SHARED_DATA_PATH='"$(pkgdatadir)"' $(AM_CFLAGS)

#EXTRA_DIST = file68.vcproj
