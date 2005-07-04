#ifndef __ARGS_H
#define __ARGS_H


class CCmdlineArgs {
public:
	CCmdlineArgs( int argc, const char** argv );

	int		find( const char* name, const char* altName = NULL ) const;
	bool	contains( const char* name, const char* altName = NULL ) const;

	const char* getString( const char* name, const char* altName = NULL ) const;
	int getInt( int def, const char* name, const char* altName = NULL ) const;
	float getFloat( float def, const char* name, const char* altName = NULL ) const;

private:
	typedef std::vector<const char*>	TStringVector;
private:
	TStringVector	mArgs;
};


inline bool CCmdlineArgs::contains( const char* name, const char* altName ) const
{
	return (find(name,altName) >= 0);
}


#endif
