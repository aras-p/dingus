#ifndef __ARGS_H
#define __ARGS_H

class CmdlineArgs {
public:
	CmdlineArgs( int argc, char** argv );
	~CmdlineArgs();

	/// Returns index or -1
	int		find( const char* name ) const;
	/// Returns true or false
	bool	contains( const char* name ) const;

	const char* getString( const char* name ) const;
	int getInt( int def, const char* name ) const;
	double getFloat( double def, const char* name ) const;

private:
	int				mCount;
	const char**	mArgs;
};


inline bool CmdlineArgs::contains( const char* name ) const
{
	return (find(name) >= 0);
}


#endif
