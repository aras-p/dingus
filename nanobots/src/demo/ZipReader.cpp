#include "stdafx.h"
#include "ZipReader.h"


char* gReadFileInZip( unzFile zipFile, const char* fileName, int& fileSize )
{
	fileSize = 0;

	// locate file
	if( unzLocateFile( zipFile, fileName, 2 ) != UNZ_OK )
		return NULL;

	// get file info
	unz_file_info fileInfo;
	if( unzGetCurrentFileInfo( zipFile, &fileInfo, NULL, 0, NULL, 0, NULL, 0 ) != UNZ_OK )
		return NULL;

	// allocate buffer for uncompressed file
	unsigned int bufSize = fileInfo.uncompressed_size;
	assert( bufSize < 50*1024*1024 );

	char* buf = new char[bufSize+1];
	if( !buf )
		return NULL;

	// open file in zip
	if( unzOpenCurrentFile( zipFile ) != UNZ_OK ) {
		delete[] buf;
		return NULL;
	}

	// read zip file
	int bytesRead = unzReadCurrentFile( zipFile, buf, bufSize );
	buf[bytesRead] = 0;

	// close file in zip
	unzCloseCurrentFile( zipFile );

	fileSize = bytesRead;
	return buf;
}


const char* gSkipUTFStart( const char* ptr, int fileSize )
{
	// skip UTF-8 header if there is one...
	if( fileSize >= 3 ) {
		if( ptr[0] == char(0xEF) && ptr[1] == char(0xBB) && ptr[2] == char(0xBF) )
			return ptr + 3;
	}
	return ptr;
}

