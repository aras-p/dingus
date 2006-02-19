// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#include "stdafx.h"
#include "StringHelper.h"

using namespace dingus;


void CStringHelper::replaceTabs( std::string& s ) {
	size_t n = s.size();
	for( size_t i = 0; i < n; ++i ) {
		if( s[i] == '\t' ) s[i] = ' ';
	}
}

void CStringHelper::replaceLineEnds( std::string& s )
{
	size_t n = s.size();
	for( size_t i = 0; i < n; ++i ) {
		if( s[i]=='\n' || s[i]=='\r' ) s[i] = ' ';
	}
}

void CStringHelper::replaceWhitespace( std::string& s )
{
	size_t n = s.size();
	for( size_t i = 0; i < n; ++i ) {
		if( !isprint(s[i]) ) s[i] = ' ';
	}
}

void CStringHelper::trimString( std::string& s ) 
{
	size_t first = s.find_first_not_of( " \t\n\r" );
	if( first == std::string::npos ) {
		s = "";
		return;
	}
	size_t last = s.find_last_not_of( " \t\n\r" );
	s = s.substr( first, (last-first+1) );
}

int CStringHelper::countLines( const std::string& s )
{
	int c = 1;
	size_t n = s.size();
	for( size_t i = 0; i < n; ++i ) {
		if( s[i]=='\n' ) ++c;
	}
	return c;
}


/*
bool CStringHelper::isFloat( const std::string& s )
{
	//[whitespace] [sign] [digits] [.digits] [ {d | D | e | E }[sign]digits]

	bool t = false;
	
	int i = 0;
	while( i < s.size() && ( s[i] == ' ' || s[i] == '\t' ) ) i++;
	if( i >= s.size() ) return t;

	if( ( s[i] == '+' ) || ( s[i] == '-' ) ) {
		i++;
		if( i >= s.size() ) return t;
	}

	if( isDigit( s[i] ) ) {
		t = true;
		while( i < s.size() && isDigit( s[i] ) ) i++;
		if( i >= s.size() ) return t;
	}

	if( s[i] == '.' ) {
		i++;
		if( i >= s.size() ) return t;

		if( isDigit( s[i] ) ) {
			t = true;
			while( i < s.size() && isDigit( s[i] ) ) i++;
			if( i >= s.size() ) return t;
		}
	}

	if( ( s[i] == 'd' ) || ( s[i] == 'D' ) || ( s[i] == 'e' ) || ( s[i] == 'E' ) ) {
		i++;
		if( i >= s.size() ) return t;

		if( ( s[i] == '+' ) || ( s[i] == '-' ) ) {
			i++;
			if( i >= s.size() ) return t;
		}

		while( i < s.size() && isDigit( s[i] ) ) i++;
		if( i >= s.size() ) return t;
	}
	return t;
}

bool CStringHelper::isInt( const std::string& s )
{
	//[whitespace] [sign] [digits] 

	bool t = false;
	
	int i = 0;
	while( i < s.size() && ( s[i] == ' ' || s[i] == '\t' ) ) i++;
	if( i >= s.size() ) return t;

	if( ( s[i] == '+' ) || ( s[i] == '-' ) ) {
		i++;
		if( i >= s.size() ) return t;
	}

	if( isDigit( s[i] ) ) {
		t = true;
		while( i < s.size() && isDigit( s[i] ) ) i++;
		if( i >= s.size() ) return t;
	}

	return t;
}
*/

bool CStringHelper::isName( const std::string& name ) 
{
	if( name.empty() )
		return false;
	if( !isLetter( name[0] ) && name[0]!='_' ) return false;
	size_t n = name.size();
	for( size_t i = 1; i < n; ++i )
		if( !( isLetter( name[i] ) || isDigit( name[i] ) || ( name[i] == '_' ) ) )
			return false;
	return true;
}

bool CStringHelper::equalsIgnoreCase( const std::string& s0, const std::string& s1 ) 
{
	if( s0.size() != s1.size() ) return false;
	std::string ss0 = s0;
	std::string ss1 = s1;
	toLower( ss0 );
	toLower( ss1 );
	return (ss0 == ss1);
}

void CStringHelper::toLower( std::string& s )
{
	size_t n = s.size();
	for( size_t i = 0; i < n; ++i )
		s[i] = (char)tolower(s[i]);
}

void CStringHelper::toUpper( std::string& s )
{
	size_t n = s.size();
	for( size_t i = 0; i < n; ++i )
		s[i] = (char)toupper(s[i]);
}

std::string CStringHelper::floatValue( float f )
{
	char buffer[256];
	sprintf( buffer, "%f", f );
	return std::string( buffer );
}

std::string CStringHelper::intValue( int i )
{
	char buffer[256];
	sprintf( buffer, "%d", i );
	return buffer;
}

std::string CStringHelper::intValue( int i, int digitsCount, bool zeros )
{
	char buffer[256];
	sprintf( buffer, "%0*d", digitsCount, i ); // TBD: zeros?
	return buffer;
}

void CStringHelper::stripPrefix( std::string& s, const std::string& prefix )
{
	if( startsWith(s,prefix) ) {
		s = s.substr( prefix.size(), s.size()-prefix.size() );
	}
}

bool CStringHelper::toBool( const std::string& str )
{
	std::string s = str;
	assert( 
		s=="true" || s=="false" || 
		s=="yes" || s=="no" || 
		s=="1" || s=="0" 
	);
	return s=="yes" || s=="true" || s=="1";
}
