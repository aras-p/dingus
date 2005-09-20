#ifndef __BYTE_UTILS_H
#define __BYTE_UTILS_H

#include "net/NetInterface.h"


namespace bu {


static inline std::string receiveStr()
{
	const BYTE* data;
	net::receiveChunk( data, 2, true ); // length
	int len = *(unsigned short*)data;

	net::receiveChunk( data, len, true );
	std::string res( (const char*)data, len );
	return res;
}

static inline int receiveInt()
{
	const BYTE* data;
	net::receiveChunk( data, 4, true );
	int res = *(const int*)data;
	return res;
}

static inline unsigned char receiveByte()
{
	const BYTE* data;
	net::receiveChunk( data, 1, true );
	unsigned char res = *(const unsigned char*)data;
	return res;
}

static inline short receiveShort()
{
	const BYTE* data;
	net::receiveChunk( data, 2, true );
	unsigned res = *(const short*)data;
	return res;
}

/*
static inline std::string readStr( const BYTE*& data )
{
	// asciiz
	//std::string res = (const char*)data;
	//data += res.length()+1;
	//return res;

	// len+data
	int len = data[0];
	std::string res( (const char*)(data+1), len );
	data += len+1;
	return res;
}
*/

static inline int readInt( const BYTE*& data )
{
	int res = *(const int*)data;
	data += 4;
	return res;
}

static inline short readShort( const BYTE*& data )
{
	short res = *(const short*)data;
	data += 2;
	return res;
}

static inline unsigned char readByte( const BYTE*& data )
{
	unsigned char res = *(const unsigned char*)data;
	data += 1;
	return res;
}


};


#endif
