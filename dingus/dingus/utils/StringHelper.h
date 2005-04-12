// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __STRING_HELPER_H
#define __STRING_HELPER_H

#include <ctype.h>

namespace dingus {

class CStringHelper {
public:
	/** Replaces tabs with single spaces. */
	static void replaceTabs( std::string& s );
	/** Replaces line ends with single spaces. */
	static void replaceLineEnds( std::string& s );
	/** Replaces white space with single spaces. */
	static void replaceWhitespace( std::string& s );

	/** Trims whitespace from ends. */
	static void trimString( std::string& s );
	
	static bool isLetter( char c ) { return isalpha(c)?true:false; }
	static bool isDigit( char c ) { return isdigit(c)?true:false; }
	///** checks is string format [whitespace] [sign] [digits] [.digits] [ {d | D | e | E }[sign]digits] */
	//static bool isFloat( const std::string& s );
	///** checks is string format [whitespace] [sign] [digits] */
	//static bool isInt( const std::string& s );
	
	/** Is valid name? Names can be letters, numbers and '_', and start with non-number. */
	static bool isName( const std::string& s );

	static bool equalsIgnoreCase( const std::string& s0, const std::string& s1 );

	static bool startsWith( const std::string& s, const std::string& subs ) {
		return s.find( subs ) == 0;
	}
	static bool endsWith( const std::string& s, const std::string& subs ) {
		return s.find( subs ) == (s.length() - subs.length());
	}
	static void stripPrefix( std::string& s, const std::string& prefix );
	
	static char toLower( char c ) { return (char)tolower(c); }
	static char toUpper( char c ) { return (char)toupper(c); }
	static void toLower( std::string& s );
	static void toUpper( std::string& s );
	
	static std::string floatValue( float f );
	static std::string intValue( int i );
	/**
	 *  Returns int to string value with added zeros or spaces in front of number.
	 *  For example if i = 15 and digitsCount = 3 then result is 015.
	 *  @param zeros Zeros if true, spaces if false.
	 */
	static std::string intValue( int i, int digitsCount, bool zeros );
	
	static std::string boolValue( bool b ) { return b ? "true" : "false"; }
	static bool toBool( const std::string& s );
};

}; // namespace

#endif
