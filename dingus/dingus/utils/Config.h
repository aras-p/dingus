// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

// config files support
#ifndef __CONFIG_H__
#define __CONFIG_H__

namespace dingus {

/**
 *	Config files support.
 *	Item in config file is represented by a name, a value and (optionally)
 *	a section it belongs. Names and sections are case sensitive. Section name is
 *	written in a separate line between '[' and ']' brackets; all items till
 *	next section belong to this section. All items before the very first section
 *	belong to all sections. Comments are written after '#' or ';' till end of line.
 */
class CConfig {
public:
	typedef std::vector<int>	TIntVector;
	typedef std::vector<float>	TFloatVector;
	typedef std::vector<std::string> TStringVector;
	typedef std::vector<bool>	TBoolVector;
public:
	// constr/destr
	CConfig( const std::string& readName = "default.cfg", bool write = false );
	CConfig( const std::string& readName, const std::string& writeName );
	~CConfig();

	// reads: string, int, float, bool respectively
	std::string  readS( const std::string& section, const std::string& name, const std::string& def );
	int 		 readI( const std::string& section, const std::string& name, int def );
	float		 readF( const std::string& section, const std::string& name, float def );
	bool		 readB( const std::string& section, const std::string& name, bool def );
	// reads many tokens, ints, floats, bool
	TStringVector	readSn( const std::string& section, const std::string& name );
	TIntVector		readIn( const std::string& section, const std::string& name );
	TFloatVector	readFn( const std::string& section, const std::string& name );
	TBoolVector		readBn( const std::string& section, const std::string& name );

	// modifies: string, int, float, bool respectively
	void modifyS( const std::string& section, const std::string& name, const std::string& val );
	void modifyI( const std::string& section, const std::string& name, int val );
	void modifyF( const std::string& section, const std::string& name, float val );
	void modifyB( const std::string& section, const std::string& name, bool val );
	// modifies: with string, int, float vectors
	//void modifySn( const char *section, const char *name, const TStringVector& vals );
	//void modifyIn( const char *section, const char *name, const TIntVector& vals );
	//void modifyFn( const char *section, const char *name, const TFloatVector& vals );

private:
	struct SLine { // one config line
		std::string 	mName; // name
		std::string 	mData; // value
		std::string 	mOther; // comments
		bool			mWrite; // false for includes
	};
	typedef std::list<SLine*>	TLineList;

private:
	// methods
	static void prepareLine( const std::string& in, std::string& name, std::string& data, std::string& other );
	static std::string prepareSection( const std::string& in );
	SLine* findString( const std::string& section, const std::string& name, TLineList::iterator* iter );
	SLine* insertLine( const TLineList::iterator& iter, const std::string& name, const std::string& data );
	void addFile( const std::string& readName, bool write );
	// data
	TLineList	mLines;
	std::string mWriteName; // will be written to
	bool	mDirty; // is modified?
	bool	mWrite; // is writing allowed?
};

}; // namespace

#endif

