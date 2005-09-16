#ifndef __BYTE_UTILS_H
#define __BYTE_UTILS_H

namespace bu {


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

static inline int readInt( const BYTE*& data )
{
	int res = *(const int*)data;
	data += 4;
	return res;
}

static inline unsigned short readShort( const BYTE*& data )
{
	unsigned short res = *(const unsigned short*)data;
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
