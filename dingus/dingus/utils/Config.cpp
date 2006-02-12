// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "Config.h"

// config files support - implementation
#include <ctype.h>	// isspace()
#include "StringHelper.h"

using namespace dingus;


/** constr/destr ***********************************************************/

CConfig::CConfig( const std::string& readName, bool write )
:	mWriteName( readName ), mDirty(false), mWrite(write)
{
	// load file
	addFile( readName, mWrite );
}

CConfig::CConfig( const std::string& readName, const std::string& writeName )
:	mWriteName( writeName ), mDirty(false), mWrite(true)
{
	// load file
	addFile( readName, mWrite );
}

CConfig::~CConfig()
{
	// write
	if( mWrite && mDirty ) {
		FILE *f = fopen( mWriteName.c_str(), "wt" );
		if (f) {
			TLineList::const_iterator it;
			for( it = mLines.begin(); it != mLines.end(); ++it ) {
				const SLine& l = **it;
				if( !l.mWrite )
					continue;
				if( !l.mName.empty() ) {
					fputs( l.mName.c_str(), f );
					if( l.mName[0] != '[' )
						fputs( " = ", f );
				}
				fputs( l.mData.c_str(), f );
				if( !l.mOther.empty() )
					fputs( " ", f );
				fputs( l.mOther.c_str(), f );
				fputs( "\n", f );
			}
			fclose(f);
		}
	}

	// destruct list
	TLineList::iterator it;
	for( it = mLines.begin(); it != mLines.end(); ++it )
		delete *it;
	mLines.clear();
}


/** loading ****************************************************************/


// makes name, data and other from in
void CConfig::prepareLine( const std::string& in, std::string& name, std::string& data, std::string& other )
{
#define SKIPWS(src) { while( i<src.size() && isspace(src[i]) ) ++i; }
#define READNWS(dest,src) { while( i<src.size() && !isspace(src[i]) ) dest += src[i++]; }
	
	name = "";
	data = "";
	other = "";
	size_t i = 0;
	SKIPWS(in);
	// read name
	if( in[i] == '[' ) { // section
		name += in[i++];
		while( i < in.size() && in[i] != ']' ) {
			SKIPWS(in);
			READNWS(name, in);
			SKIPWS(in);
		}
	} else { // ordinary name
		while( i<in.size() && !isspace(in[i]) && in[i]!='=' && in[i]!='#' && in[i]!=';' )
			name += in[i++];
	}

	// skip ws and '='
	while( i<in.size() && ( isspace(in[i]) || in[i]=='=' ) )
		i++;
	
	// read data
	if( in[i] == '\"' )
		i++;
	while( i<in.size() && in[i]!='#' && in[i]!=';' )
		data += in[i++];
	if( !data.empty() && (data[data.size()-1]=='\"' || isspace(data[data.size()-1]) ) )
		data.resize( data.size()-1 );
		
	// read other
	while( i<in.size() && in[i]!='\n' && in[i]!='\r' )
		other += in[i++];
	if( !other.empty() && isspace(other[other.size()-1]) )
		other.resize( other.size()-1 );
}


// reads file
void CConfig::addFile( const std::string& readName, bool write )
{
	std::string line, name, data, other;
	FILE *f;
	SLine *p;
	
	if( readName.empty() )
		return;
	f = fopen( readName.c_str(), "rt" );
	if( !f )
		return;
	
	for(;;) {
		// read a line
		char buf[256];
		if( !fgets( buf, 256, f ) )
			break;
		line = buf;
		
		prepareLine( line, name, data, other );

		// make a line structure
		p = new SLine();
		p->mName = name;
		p->mData = data;
		p->mOther = other;
		p->mWrite = write;

		// manage list
		mLines.push_back( p );

		// check for "INCLUDE"
		if( p->mName == "INCLUDE" )
			addFile( p->mData.c_str(), false );
	}
	fclose( f );
}

/** parsing ****************************************************************/

// makes section name between '[' and ']'
std::string CConfig::prepareSection( const std::string& in )
{
	std::string res;
	if( !in.empty() ) {
		if( in[0] != '[' )
			res += '[';
		res += in;
		if( in[in.size()-1] != ']' )
			res += ']';
	}
	return res;
}

// finds smth in config
CConfig::SLine* CConfig::findString( const std::string& section, const std::string& name, TLineList::iterator* iter )
{
	bool goodSect = true;
	
	std::string sectName = prepareSection( section );
	TLineList::iterator it = mLines.begin();
	while( it != mLines.end() ) {
		SLine& l = **it;
		if( !l.mName.empty() ) {
			// section
			if( l.mName[0]=='[' && l.mName[l.mName.size()-1]==']' )
				goodSect = (l.mName == sectName);
			// check if what is needed
			if( goodSect || name[0] == '[' )
				if( l.mName == name ) {
					if( iter )
						*iter = it;
					return &l;
				}
		}
		++it;
	}
	if( iter )
		*iter = it;
	return NULL;
}


std::string CConfig::readS( const std::string& section, const std::string& name, const std::string& def )
{
	SLine *p = findString( section, name, NULL );
	return p ? p->mData : def;
}

int CConfig::readI( const std::string& section, const std::string& name, int def )
{
	SLine *p = findString( section, name, NULL );
	return p ? (int)strtol( p->mData.c_str(), NULL, 0 ) : def;
}

float CConfig::readF( const std::string& section, const std::string& name, float def )
{
	SLine *p = findString( section, name, NULL );
	return p ? (float)strtod( p->mData.c_str(), NULL ) : def;
}

bool CConfig::readB( const std::string& section, const std::string& name, bool def )
{
	SLine *p = findString( section, name, NULL );
	return p ? CStringHelper::toBool( p->mData ) : def;
}

CConfig::TStringVector CConfig::readSn( const std::string& section, const std::string& name )
{
	TStringVector res;
	std::string s = readS( section, name, "" );

	size_t i = 0;
	while( i < s.size() ) {
		while( i<s.size() && isspace(s[i]) )
			++i;
		if( i >= s.size() )
			break;
		std::string ss;
		while( i<s.size() && !isspace(s[i]) )
			ss += s[i++];
		res.push_back( ss );
	}
	return res;
}

CConfig::TIntVector CConfig::readIn( const std::string& section, const std::string& name )
{
	TIntVector res;
	std::string s = readS( section, name, "" );

	size_t i = 0;
	while( i < s.size() ) {
		while( i<s.size() && isspace(s[i]) )
			++i;
		if( i >= s.size() )
			break;
		std::string ss;
		while( i<s.size() && !isspace(s[i]) )
			ss += s[i++];
		res.push_back( (int)strtol( ss.c_str(), NULL, 0 ) );
	}
	return res;
}

CConfig::TFloatVector CConfig::readFn( const std::string& section, const std::string& name )
{
	TFloatVector res;
	std::string s = readS( section, name, "" );

	size_t i = 0;
	while( i < s.size() ) {
		while( i<s.size() && isspace(s[i]) )
			++i;
		if( i >= s.size() )
			break;
		std::string ss;
		while( i<s.size() && !isspace(s[i]) )
			ss += s[i++];
		res.push_back( (float)strtod( ss.c_str(), NULL ) );
	}
	return res;
}

CConfig::TBoolVector CConfig::readBn( const std::string& section, const std::string& name )
{
	TBoolVector res;
	std::string s = readS( section, name, "" );

	size_t i = 0;
	while( i < s.size() ) {
		while( i<s.size() && isspace(s[i]) )
			++i;
		if( i >= s.size() )
			break;
		std::string ss;
		while( i<s.size() && !isspace(s[i]) )
			ss += s[i++];
		res.push_back( CStringHelper::toBool( ss ) );
	}
	return res;
}


/** modifying **************************************************************/

// inserts new line after iter
CConfig::SLine* CConfig::insertLine( const TLineList::iterator& iter, const std::string& name, const std::string& data )
{
	SLine *n = new SLine;
	n->mName = name;
	n->mData = data;
	n->mOther = "";
	n->mWrite = true;

	if( iter == mLines.end() )
		mLines.push_front( n );
	else {
		TLineList::iterator iter2 = iter;
		++iter2;
		if( iter2 == mLines.end() )
			mLines.push_back( n );
		else
			mLines.insert( iter2, n );
	}
	return n;
}

// modifies a config line
void CConfig::modifyS( const std::string& section, const std::string& name, const std::string& val )
{
	// check if it exists
	TLineList::iterator iter;
	SLine *p = findString( section, name, &iter );
	if( p ) {
		// if a new value is given - modify
		if( !val.empty() )
			p->mData = val;
		else {
			delete p;
			mLines.erase( iter );
		}
	// doesn't exist: if value is given - create a new one
	} else if( !val.empty() ) {
		std::string sectName = prepareSection( section );
		// insert into specific section
		if( !sectName.empty() ) {
			p = findString( "", sectName.c_str(), &iter );
			// section doesn't exist - create new section
			if( !p ) {
				insertLine( iter, "", "" );
				++iter;
				insertLine( iter, sectName.c_str(), "" );
				++iter;
			}
			insertLine( iter, name, val.c_str() );
		} else // insert into common section
			insertLine( mLines.end(), name, val.c_str() );
	}
	mDirty = true; // modified
}

void CConfig::modifyI( const std::string& section, const std::string& name, int val )
{
	char buf[64];
	sprintf(buf, "%i", val);
	CConfig::modifyS( section, name, buf );
}

void CConfig::modifyF( const std::string& section, const std::string& name, float val )
{
	char buf[64];
	sprintf(buf, "%f", val);
	CConfig::modifyS( section, name, buf );
}

void CConfig::modifyB( const std::string& section, const std::string& name, bool val )
{
	CConfig::modifyS( section, name, CStringHelper::boolValue( val ) );
}

/*void CConfig::modifySn( const char *section, const char *name, int num, const char **val )
{
	char buf[256];
	int bufN = 0;
	for (int i = 0; i < num; i++) {
		int l = strlen( val[i] );
		if (l + bufN >= 254) break;
		memcpy( buf + bufN, val[i], l );
		bufN += l;
		buf[ bufN ] = ' ';
		bufN++;
	}
	buf[ bufN ] = 0;
	modifyS( section, name, buf );
}

void CConfig::modifyIn( const char *section, const char *name, int num, const int *val )
{
	char buf[256];
	int bufN = 0;
	for (int i = 0; i < num; i++) {
		bufN += sprintf( buf + bufN, "%i ", val[i]);
		if (bufN >= 200) break;
	}
	modifyS( section, name, buf );
}

void CConfig::modifyFn( const char *section, const char *name, int num, const float *val )
{
	char buf[256];
	int bufN = 0;
	for (int i = 0; i < num; i++) {
		bufN += sprintf( buf + bufN, "%f ", val[i]);
		if (bufN >= 180) break;
	}
	modifyS( section, name, buf );
}
*/