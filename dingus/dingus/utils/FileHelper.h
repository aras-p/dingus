// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __FILE_HELPER_H
#define __FILE_HELPER_H

namespace dingus {

class CFileHelper {
public:
	typedef std::list<std::string> TStringList;

public:
	/** Returns list of file names that are in given path. */
	static TStringList getFiles( const std::string& path, bool recursive, const std::string& wildcard = "*" );

	//static std::string getNameSpace( const std::string& path, const std::string& subPath );

	//static std::string getFilePath( const std::string& path, const std::string& fileName, const std::string& nameSpace );

	//static bool equalsFilePaths( const std::string& filePath1, const std::string& filePath2 );
	
private:
	//static bool isNonListing( const std::string& path );
};

}; // namespace

#endif
