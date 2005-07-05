#ifndef __ARGS_H
#define __ARGS_H

#include <string>
#include <vector>


class CCmdlineArgs {
public:
	CCmdlineArgs( int argc, const char** argv );

	int		find( const char* name ) const;
	bool	contains( const char* name ) const;

	const char* getString( const char* name ) const;
	int getInt( int def, const char* name ) const;
	float getFloat( float def, const char* name ) const;

private:
	typedef std::vector<const char*>	TStringVector;
private:
	TStringVector	mArgs;
};


inline bool CCmdlineArgs::contains( const char* name ) const
{
	return (find(name) >= 0);
}


#endif
