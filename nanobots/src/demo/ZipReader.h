#ifndef __ZIP_READER_H
#define __ZIP_READER_H

#include <zlib/unzip.h>


char* gReadFileInZip( unzFile zipFile, const char* fileName, int& fileSize );

const char* gSkipUTFStart( const char* ptr, int fileSize );


#endif
