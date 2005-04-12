// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __GFX_MODEL_DESC_SERIALIZER_H
#define __GFX_MODEL_DESC_SERIALIZER_H

#include "ModelDesc.h"


namespace dingus {


class CModelDescSerializer {
public:
	/// Return true if successful.
	static bool loadFromFile( const char* fileName, CModelDesc& desc );
	/// Return true if successful
	static bool saveToFile( const char* fileName, const CModelDesc& desc );
};


}; // namespace


#endif
