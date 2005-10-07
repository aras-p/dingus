#include "stdafx.h"
#include "EntityManager.h"
#include "AttackEntity.h"
#include "../GameInfo.h"
#include "../game/GameState.h"
#include "../game/GameDesc.h"
#include "../MinimapRenderer.h"
#include "../EntityInfoRenderer.h"
#include "../game/GameColors.h"

#include <dingus/gfx/DebugRenderer.h>
#include <dingus/gfx/gui/Gui.h>


const float OUTLINE_TTL = 0.25f;


CEntityManager::CEntityManager()
:	mLastMouseEntityID(-1),
	mSelectedEntityID(-1)
{
	mAttackManager = new CAttackEntityManager();

	CGameInfo& gi = CGameInfo::getInstance();
	const CGameDesc& desc = gi.getGameDesc();
	const CGameMap& gmap = desc.getMap();

	int i, n;

	// TBD: hollow needles on non-occupied hoshimi points

	// point entities
	n = gmap.getPointCount();
	mPointEntities.reserve( n + G_MAX_PLAYERS ); // reserve space for injection points
	mPointEntities.resize( n ); 
	for( i = 0; i < n; ++i ) {
		mPointEntities[i] = new CPointEntity( gmap.getPoint(i) );
	}

	// stream entities
	const float STREAM_ENT_CELL_STRIDE = 3.0f;

	n = gmap.getStreamCount();
	mStreamEntities.reserve( 128 );
	for( i = 0; i < n; ++i ) {
		const CGameMap::SStream& strm = gmap.getStream(i);
		for( float sy = strm.y; sy < strm.y+strm.height; sy += STREAM_ENT_CELL_STRIDE ) {
			for( float sx = strm.x; sx < strm.x+strm.width; sx += STREAM_ENT_CELL_STRIDE ) {
				CStreamEntity* e = new CStreamEntity( strm, sx, sy );
				mStreamEntities.push_back( e );
			}
		}
	}
}

CEntityManager::~CEntityManager()
{
	stl_utils::wipe_map( mActorEntities );
	stl_utils::wipe( mPointEntities );
	stl_utils::wipe( mStreamEntities );
	delete mAttackManager;
}

void CEntityManager::onMouseClick()
{
	mSelectedEntityID = mLastMouseEntityID;
}

void CEntityManager::update( const SLine3& mouseRay, float timeAlpha )
{
	int i, n;

	// check mouse intersection
	mLastMouseEntityID = getCollidedEntityID( mouseRay );
	CActorEntity* selEntity = getActorEntityByID( mLastMouseEntityID );
	if( selEntity != NULL ) {
		selEntity->setOutline( OUTLINE_TTL );
	} else {
		mLastMouseEntityID = -1;
	}

	// update actor entities
	TActorEntityMap::iterator it, itEnd = mActorEntities.end();
	for( it = mActorEntities.begin(); it != itEnd; ++it ) {
		CActorEntity& e = *it->second;
		e.update( timeAlpha );
		if( !e.getGameEntity().isAlive() ) {
			if( mSelectedEntityID == e.getGameEntity().getID() )
				mSelectedEntityID = -1;
			continue;
		}
	}

	// update point entities
	n = mPointEntities.size();
	for( i = 0; i < n; ++i ) {
		CPointEntity& e = *mPointEntities[i];
		e.update();
	}

	// update stream entities
	n = mStreamEntities.size();
	for( i = 0; i < n; ++i ) {
		CStreamEntity& e = *mStreamEntities[i];
		e.update();
	}

	// set max. outline for selected entity
	selEntity = getActorEntityByID( mSelectedEntityID );
	if( selEntity ) {
		selEntity->setOutline( OUTLINE_TTL );
	} else {
		mSelectedEntityID = -1;
	}
}


void CEntityManager::renderMinimap()
{
	CGameInfo& gi = CGameInfo::getInstance();
	const CGameMap& gmap = gi.getGameDesc().getMap();

	int i, n;

	CMinimapRenderer& minir = gi.getMinimapRenderer();
	minir.beginEntities();

	// camera
	SVector3 campos = G_RENDERCTX->getCamera().getEye3();
	campos.y = 10.0f;
	minir.addEntity( campos, 0x80ffff00, 4.0f );
	// special points
	n = gmap.getPointCount();
	for( i = 0; i < n; ++i ) {
		const CGameMap::SPoint& pt = gmap.getPoint(i);
		minir.addEntity( SVector3(pt.x,0.0f,-pt.y), pt.colorMain & 0x80ffffff, 4.0f );
	}
	// actors
	TActorEntityMap::iterator it, itEnd = mActorEntities.end();
	for( it = mActorEntities.begin(); it != itEnd; ++it ) {
		CActorEntity& e = *it->second;
		if( e.getGameEntity().isAlive() )
			minir.addEntity( e.mWorldMat.getOrigin(), e.getColorMinimap(), e.getGameEntity().getType()==ENTITY_AI ? 2.0f : 1.0f );
		// render attack?
		const CGameEntity::SState& s = e.getGameEntity().getState();
		if( s.state == ENTSTATE_ATTACK )
			minir.addEntity( SVector3(s.targx,0.0f,-s.targy), 0xFFff0000, 3.0f );
		//const CReplayEntity::SState& ss = e.getReplayEntity().getTurnState( turn-1 );
		//if( ss.state == ENTSTATE_ATTACK )
		//	minir.addEntity( SVector3(ss.targx,0.0f,-ss.targy), 0xFFff0000, 3.0f );
	}
	minir.endEntities();
	minir.render();
}


void CEntityManager::render( eRenderMode rm, bool entityBlobs, bool thirdPerson )
{
	CGameInfo& gi = CGameInfo::getInstance();

	int i, n;

	CMinimapRenderer& blobr = gi.getEntityBlobsRenderer();
	CEntityInfoRenderer& eir = gi.getEntityInfoRenderer();

	const SMatrix4x4& cameraMat = G_RENDERCTX->getCamera().getCameraMatrix();
	const float barSize = entityBlobs ? 0.1f : 0.06f;

	blobr.beginEntities();
	eir.beginInfos();
	mAttackManager->begin();

	// actors
	const int lodOffset = GFX_DETAIL_LEVELS-1 - gAppSettings.gfxDetail;

	TActorEntityMap::iterator it, itEnd = mActorEntities.end();
	for( it = mActorEntities.begin(); it != itEnd; ++it ) {
		CActorEntity& e = *it->second;
		// explosion?
		// TBD
		/*
		float deathA = e.getReplayEntity().getDeathAlpha( t );
		if( deathA >= 0.0f ) {
			mAttackManager->renderExplosion( e.mWorldMat.getOrigin(), deathA );
		}

		if( !e.isAlive() && (deathA<0.0f || deathA>0.3f) )
			continue;
		*/
		if( !e.getGameEntity().isAlive() )
			continue;

		// calculate LOD
		float camdist = cameraMat.getAxisZ().dot( e.mWorldMat.getOrigin() - cameraMat.getOrigin() );
		int lod = CActorEntity::LOD_COUNT-1;
		if( camdist >= 0.0f ) {
			lod = camdist * 0.05f + lodOffset;
			if( lod > CActorEntity::LOD_COUNT-1 )
				lod = CActorEntity::LOD_COUNT-1;
		}
		

		e.render( rm, lod, false );
		if( entityBlobs ) {
			blobr.addEntity( e.mWorldMat.getOrigin() + SVector3(0,e.getBlobDY(),0), e.getColorBlob(), 1.0f );
		}

		int owner = e.getGameEntity().getOwner();

		// health bar / outline
		const CGameEntity::SState& s = e.getGameEntity().getState();
		float healthBar = float(s.health) / e.getGameEntity().getMaxHealth();
		if( camdist >= 0.0f ) {
			// fog the health bar
			float fog = clamp( (gFogParam.y - camdist) * gFogParam.z, 0.0f, 1.0f );
			D3DCOLOR barColor = gColors.team[owner].tone.c;
			barColor &= 0x00ffffff;
			barColor |= int(fog*255.0f) << 24;

			float size = sqrtf(camdist) * barSize;
			eir.addBar( e.mWorldMat.getOrigin() - cameraMat.getAxisZ()*0.35f, e.getHealthBarDY(), barColor, healthBar, size );
			
			// outline
			float outline = e.getOutline();
			if( outline > 0.0f ) {
				D3DCOLOR outcolor = 0x00ffffff;
				outcolor |= int((outline/OUTLINE_TTL)*160.0f) << 24;
				eir.addBracket( e.mWorldMat.getOrigin(), e.getOutlineDY(), outcolor, thirdPerson ? 0.6f : 1.5f );
			}
		}

		// render AZN nimbus?
		if( s.stock > 0 ) {
			SVector3 nimbusPos = e.mWorldMat.getOrigin();
			nimbusPos.y += e.getOutlineDY();
			float nimbusScale = (e.getGameEntity().getType() == ENTITY_NEEDLE) ? 1.0f : 0.6f;
			mAttackManager->renderNimbus( nimbusPos, nimbusScale );
		}

		// render attack?
		const SVector4& atkColor = gColors.team[owner].main.v;
		if( s.state == ENTSTATE_ATTACK ) {
			mAttackManager->renderAttack( e.mWorldMat.getOrigin(), s.targx, s.targy, /*turnAlpha*0.5f TBD*/0.5f, atkColor );
		}
		/*
		const CReplayEntity::SState& ss = e.getReplayEntity().getTurnState( turn-1 );
		if( ss.state == ENTSTATE_ATTACK ) {
			mAttackManager->renderAttack( e.mWorldMat.getOrigin(), ss.targx, ss.targy, 0.5f+turnAlpha*0.5f, atkColor );
		}
		*/
	}

	mAttackManager->end();
	blobr.endEntities();
	eir.endInfos();
	blobr.render();
	eir.render();

	// points
	n = mPointEntities.size();
	for( i = 0; i < n; ++i ) {
		mPointEntities[i]->renderPoint( rm );
	}

	// streams
	n = mStreamEntities.size();
	for( i = 0; i < n; ++i ) {
		mStreamEntities[i]->render( rm );
	}
}



void CEntityManager::renderLabels( CUIDialog& dlg )
{
	if( !gAppSettings.drawAznCollector && !gAppSettings.drawAznNeedle )
		return;

	CGameInfo& gi = CGameInfo::getInstance();

	const SMatrix4x4& cameraMat = G_RENDERCTX->getCamera().getCameraMatrix();

	char buf[100];

	TActorEntityMap::iterator it, itEnd = mActorEntities.end();
	for( it = mActorEntities.begin(); it != itEnd; ++it ) {
		CActorEntity& e = *it->second;

		// dead?
		if( !e.getGameEntity().isAlive() )
			continue;

		// figure out what should we draw
		eEntityType etype = e.getGameEntity().getType();
		const CGameEntity::SState& s = e.getGameEntity().getState();
		
		bool drawAzn = false;
		bool drawName = gAppSettings.drawEntityNames;
		float maxNameAlpha = drawName ? 0.7f : 0.0f;

		if( etype == ENTITY_NEEDLE && gAppSettings.drawAznNeedle )
			drawAzn = true;
		if( etype == ENTITY_COLLECTOR && gAppSettings.drawAznCollector && s.stock > 0 )
			drawAzn = true;

		if( mSelectedEntityID == e.getGameEntity().getID() ) {
			drawName = true;
			maxNameAlpha = 1.0f;
		} else if( e.getOutline() >= 0 ) {
			drawName = true;
			float outline = e.getOutline()/OUTLINE_TTL;
			if( maxNameAlpha < outline )
				maxNameAlpha = outline;
		}

		if( !drawAzn && (!drawName || s.name[0]==0) )
			continue;

		// behind camera?
		float camdist = cameraMat.getAxisZ().dot( e.mWorldMat.getOrigin() - cameraMat.getOrigin() );
		if( camdist < 0.0f )
			continue;

		// calculate fade-out for text
		float fogfar, fogfader, maxAznAlpha;
		float fogfarName = gFogParam.y, fogfaderName = gFogParam.z;
		if( etype == ENTITY_NEEDLE ) {
			fogfar = gFogParam.y;
			fogfader = gFogParam.z;
			maxAznAlpha = 1.0f;
		} else {
			fogfar = (gFogParam.x + gFogParam.y) * 0.5f;
			fogfader = gFogParam.z * 0.75f;
			maxAznAlpha = 0.5f;
		}
		float fog = clamp( (fogfar - camdist) * fogfader, 0.0f, maxAznAlpha );
		float fogName = clamp( (fogfarName - camdist) * fogfaderName, 0.0f, maxNameAlpha );
		if( fog <= 0.0f && fogName <= 0.0f )
			continue; // don't draw invisible texts

		const SVector4& color = gColors.team[e.getGameEntity().getOwner()].tone.v;
		D3DXCOLOR colorDx;
		colorDx.r = color.x;
		colorDx.g = color.y;
		colorDx.b = color.z;
		colorDx.a = fog;

		// project the text
		SVector3 entityPos = e.mWorldMat.getOrigin() + cameraMat.getAxisX()*0.5f + cameraMat.getAxisY()*0.5f;
		SVector3 screenPos;
		D3DXVec3TransformCoord( &screenPos, &entityPos, &G_RENDERCTX->getCamera().getViewProjMatrix() );
		screenPos.x = screenPos.x * 0.5f + 0.5f;
		screenPos.y = -screenPos.y * 0.5f + 0.5f;

		SFRect textRC;
		textRC.left = screenPos.x * GUI_X;
		textRC.right = screenPos.x * GUI_X + 100;
		textRC.top = screenPos.y * GUI_Y;
		textRC.bottom = screenPos.y * GUI_Y + 20;

		// draw azn label
		if( drawAzn ) {
			itoa( s.stock, buf, 10 );
			dlg.imDrawText( buf, 0, DT_LEFT, colorDx, textRC, false );
		}
		// draw name
		if( drawName ) {
			colorDx.a = fogName;
			textRC.top -= 12;
			textRC.bottom -= 12;
			dlg.imDrawText( s.name, 0, DT_LEFT, colorDx, textRC, false );
		}
	}
}


int CEntityManager::getCollidedEntityID( const SLine3& ray ) const
{
	int index = -1;
	float minDist = DINGUS_BIG_FLOAT;

	TActorEntityMap::const_iterator it, itEnd = mActorEntities.end();
	for( it = mActorEntities.begin(); it != itEnd; ++it ) {
		const CActorEntity& e = *it->second;
		if( !e.getGameEntity().isAlive() )
			continue;
		const SVector3& pos = e.mWorldMat.getOrigin();
		if( ray.project( pos ) < 0.0f )
			continue;
		if( ray.distance( pos ) > 1.0f )
			continue;
		float dist2 = SVector3(pos-ray.pos).lengthSq();
		if( dist2 < minDist ) {
			index = it->first;
			minDist = dist2;
		}
	}
	return index;
}


void CEntityManager::onNewGameEntity( const CGameEntity& e )
{
	assert( getActorEntityByID(e.getID()) == NULL );
	CActorEntity* actor = new CActorEntity( e );
	mActorEntities.insert( std::make_pair( e.getID(), actor ) );
}

void CEntityManager::onNewInjectionPoint( const CGameMap::SPoint& pt )
{
	mPointEntities.push_back( new CPointEntity(pt) );
}
