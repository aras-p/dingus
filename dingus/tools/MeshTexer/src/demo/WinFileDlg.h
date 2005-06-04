#ifndef __WIN_FILE_DLGS_H
#define __WIN_FILE_DLGS_H

#include <commdlg.h>

#define FILE_FILTER_MESH "All files\0*.*\0Mesh files (*.dmesh;*.x)\0*.dmesh;*.x\0"
#define FILE_FILTER_TEX  "All files\0*.*\0Texture files (*.dds;*.png;*.jpg;*.tga;*.bmp)\0*.dds;*.png;*.jpg;*.tga;*.bmp\0"


std::string gGetOpenFileName( const char* fileFilter )
{
	OPENFILENAME ofn;
	char szFile[260];
	ZeroMemory( szFile, sizeof(szFile) );

	ZeroMemory( &ofn, sizeof(OPENFILENAME) );
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = fileFilter;
	ofn.nFilterIndex = 2;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = ".";
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	if( GetOpenFileName(&ofn)==TRUE ) {
		return szFile;
	}
	return "";
}


#endif
