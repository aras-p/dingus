// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "stdafx.h"

#include "ModelDescSerializer.h"
#include "../lua/LuaSingleton.h"
#include "../lua/LuaHelper.h"
#include "../lua/LuaIterator.h"

using namespace dingus;

bool CModelDescSerializer::loadFromFile( const char* fileName, CModelDesc& desc )
{
	// clear desc...
	desc.setMeshID( "" );
	desc.clearGroups();

	// execute file
	CLuaSingleton& lua = CLuaSingleton::getInstance();
	int errorCode = lua.doFile( fileName, false );
	if( errorCode )
		return false; // error

	// create desc
	std::string name = lua.getGlobal("name").getString();
	lua.discard();
	desc.setMeshID( name );

	// read groups
	CLuaValue luaGroups = lua.getGlobal("groups");
	CLuaArrayIterator itGrps( luaGroups );
	while( itGrps.hasNext() ) {
		CLuaValue& luaGrp = itGrps.next();

		// add group
		std::string fx = CLuaHelper::getString( luaGrp, "fx" );
		int pri = (int)CLuaHelper::getNumber( luaGrp, "pri" );
		desc.addGroup( pri, fx );

		// group params
		const int grp = desc.getGroupCount() - 1;
		CLuaValue luaParams = luaGrp.getElement("params");
		CLuaArrayIterator itParams( luaParams );
		while( itParams.hasNext() ) {
			CLuaValue& luaPar = itParams.next();
			std::string ttype = luaPar.getElement(1).getString();
			std::string tname = luaPar.getElement(2).getString();
			if( ttype == "tex" ) {
				std::string tid = luaPar.getElement(3).getString();
				desc.addParamTexture( grp, tname, tid );
				luaPar.discard();
			} else if( ttype == "cube" ) {
				std::string tid = luaPar.getElement(3).getString();
				desc.addParamCubemap( grp, tname, tid );
				luaPar.discard();
			} else if( ttype == "stex" ) {
				std::string tid = luaPar.getElement(3).getString();
				desc.addParamSTexture( grp, tname, tid );
				luaPar.discard();
			} else if( ttype == "vec3" ) {
				SVector3 v;
				v.x = float( luaPar.getElement(3).getNumber() );
				v.y = float( luaPar.getElement(4).getNumber() );
				v.z = float( luaPar.getElement(5).getNumber() );
				desc.addParamVec3( grp, tname, v );
				luaPar.discard(); luaPar.discard(); luaPar.discard();
			} else if( ttype == "vec4" ) {
				SVector4 v;
				v.x = float( luaPar.getElement(3).getNumber() );
				v.y = float( luaPar.getElement(4).getNumber() );
				v.z = float( luaPar.getElement(5).getNumber() );
				v.w = float( luaPar.getElement(6).getNumber() );
				desc.addParamVec4( grp, tname, v );
				luaPar.discard(); luaPar.discard(); luaPar.discard(); luaPar.discard();
			} else if( ttype == "flt" ) {
				float v = float( luaPar.getElement(3).getNumber() );
				desc.addParamFloat( grp, tname, v );
				luaPar.discard();
			} else {
				ASSERT_FAIL_MSG( "Unsupported param type!" );
			}
			luaPar.discard();
			luaPar.discard();
		}
		luaParams.discard();
	}
	luaGroups.discard();

	return true;
}

bool CModelDescSerializer::saveToFile( const char* fileName, const CModelDesc& desc )
{
	FILE *f = fopen( fileName, "wt" );
	if( !f )
		return false; // error!

	// name
	fprintf( f, "name = '%s'\n", desc.getMeshID().getUniqueName().c_str() );

	// groups
	int n = desc.getGroupCount();
	fprintf( f, "groups = {\n" );
	for( int i = 0; i < n; ++i ) {
		fprintf( f, "{\n" );
		// fx, priority
		fprintf( f, "\tfx = '%s',\n", desc.getFxID(i).getUniqueName().c_str() );
		fprintf( f, "\tpri = %i,\n", desc.getRenderPriority(i) );
		// params
		const CModelDesc::SParams& p = desc.getParams(i);
		fprintf( f, "\tparams = {\n" );
		int j;
		for( j = 0; j < p.textures.size(); ++j ) {
			const CModelDesc::SParams::TNameIDPair& e = p.textures[j];
			fprintf( f, "\t\t{ 'tex', '%s', '%s' },\n", e.first.c_str(), e.second.c_str() );
		}
		for( j = 0; j < p.cubemaps.size(); ++j ) {
			const CModelDesc::SParams::TNameIDPair& e = p.cubemaps[j];
			fprintf( f, "\t\t{ 'cube', '%s', '%s' },\n", e.first.c_str(), e.second.c_str() );
		}
		for( j = 0; j < p.stextures.size(); ++j ) {
			const CModelDesc::SParams::TNameIDPair& e = p.stextures[j];
			fprintf( f, "\t\t{ 'stex', '%s', '%s' },\n", e.first.c_str(), e.second.c_str() );
		}
		for( j = 0; j < p.vectors3.size(); ++j ) {
			const CModelDesc::SParams::TNameVec3Pair& e = p.vectors3[j];
			const SVector3& v = e.second;
			fprintf( f, "\t\t{ 'vec3', '%s', %g, %g, %g },\n", e.first.c_str(), v.x, v.y, v.z );
		}
		for( j = 0; j < p.vectors4.size(); ++j ) {
			const CModelDesc::SParams::TNameVec4Pair& e = p.vectors4[j];
			const SVector4& v = e.second;
			fprintf( f, "\t\t{ 'vec4', '%s', %g, %g, %g, %g },\n", e.first.c_str(), v.x, v.y, v.z, v.w );
		}
		for( j = 0; j < p.floats.size(); ++j ) {
			const CModelDesc::SParams::TNameFloatPair& e = p.floats[j];
			fprintf( f, "\t\t{ 'flt', '%s', %g },\n", e.first.c_str(), e.second );
		}
		fprintf( f, "\t},\n" );
		fprintf( f, "},\n" );
	}
	fprintf( f, "}\n" );

	fclose( f );
	return true;
}


