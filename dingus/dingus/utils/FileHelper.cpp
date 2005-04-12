// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "FileHelper.h"

#include "StringHelper.h"
#include <io.h>

using namespace dingus;


CFileHelper::TStringList CFileHelper::getFiles( const std::string& p, bool recursive, const std::string& wildcard )
{
	std::string path = p; 

	// add '/' to end if there isn't yet
	if( !path.empty() && path[path.size()-1]!='\\' && path[path.size()-1]!='/' )
		path += "/";

	TStringList files;

	// non-listing directory?
	//if( isNonListing( path ) ) return files;

	// find first
	_finddata_t file;
	int handle = _findfirst( (path+wildcard).c_str(), &file );
	int ret = handle;

	// while found
	while( ret >= 0 ) {
		if( !( file.attrib & _A_SUBDIR ) ) {
			files.push_back( path + file.name );
		} else if( recursive ) {
			std::string subDir( file.name );

			CStringHelper::toLower( subDir );
			
			if( subDir!="." && subDir!=".." && subDir!="cvs" ) {
				TStringList l = getFiles( path + subDir, true );
				files.splice( files.end(), l );
			}
		}

		ret = _findnext( handle, &file );
	}

	return files;
}

/*
std::string CFileHelper::getNameSpace( const std::string& path, const std::string& filePath )
{
	assert( path.size() <= filePath.size() || !"file path should be longer than path" );

	std::string sub = filePath.substr( path.size(), filePath.size() - path.size() );

	for( int i = sub.size() - 1; i >= 0; i-- )
		if( ( sub[i] == '/' ) || ( sub[i] == '\\' ) ) break;

	if( i < 0 ) sub = "";
	else sub = sub.substr( 0, i );

	std::string nameSpace( "" );

	for( i = 0; i < sub.size(); i++ )
		if( ( sub[i] != '/' ) && ( sub[i] != '\\' ) ) nameSpace += sub[i];
		else nameSpace += "::";

	return nameSpace;
}	
*/
/*
bool CFileHelper::isNonListing( const std::string& path )
{
	std::string filePattern = path + "exclude";

	_finddata_t file;
	int handle = _findfirst( filePattern.data(), &file );
	int ret = handle;

	return ret >= 0;
}
*/

/*
std::string CFileHelper::getFilePath( const std::string& path, const std::string& fileName, const std::string& nameSpace )
{
	std::string filePath = path;
	if( filePath[filePath.size() - 1] != '\\' && filePath[filePath.size() - 1] != '/' ) filePath += '/';

	if( nameSpace.size() > 0 ) {
		for( int i = 0; i < nameSpace.size(); i++ )
			if( nameSpace[i] != ':' ) filePath += nameSpace[i];
			else {
				filePath += '/';
				i++;
				CHECK_ERROR( ( i < nameSpace.size() && nameSpace[i] == ':' ), "wrong name space name (" + nameSpace + ")" );
			}

		CHECK_ERROR( ( filePath[filePath.size() - 1] == '/' ), "wrong name space name (" + nameSpace + ")" );

		filePath += '/';
	}

	filePath += fileName;

	return filePath;
}
*/

/*
bool CFileHelper::equalsFilePaths( const std::string& filePath1, const std::string& filePath2 )
{
	if( filePath1.size() != filePath2.size() ) return false;

	for( int i = 0; i < filePath1.size(); i++ )
		if( ( CStringHelper::toLower( filePath1[i] ) != CStringHelper::toLower( filePath2[i] ) ) &&
			!(( filePath1[i] == '/' || filePath1[i] == '\\' ) &&
			  ( filePath2[i] == '/' || filePath2[i] == '\\' )) ) return false;

	return true;
}
*/