#ifndef __POST_PROCESS_H
#define __POST_PROCESS_H

#include <dingus/renderer/RenderableMesh.h>


class CPostProcess {
public:
	CPostProcess( const char* ridTemp1, const char* ridTemp2/*, const char* ridDownSmp*/ );
	~CPostProcess();
	
	// down-samples into temp1
	void	downsampleRT( IDirect3DSurface9& rt );

	// Ping-pong blurs between temp 1 and 2
	void	pingPongBlur( int passes, int startPass = 0, float spreadMult = 1.0f );

private:
	std::string			mTempRids[2];
	CRenderableMesh*	mBloomPingPongs[2];
	SVector4			mFixUVs4th;
	SVector4			mTexelOffsetsX;
	SVector4			mTexelOffsetsY;

	//CRenderableMesh*	mDownsample4;
};


#endif
