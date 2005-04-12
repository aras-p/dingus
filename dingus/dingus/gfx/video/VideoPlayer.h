// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __VIDEO_PLAYER_H
#define __VIDEO_PLAYER_H

#include <dshow.h>
#include "../../kernel/Proxies.h"
#include "../../resource/DeviceResource.h"

namespace dingus {

// --------------------------------------------------------------------------

class CDShowTextureRenderer;

class CVideoPlayer : public boost::noncopyable, public IDeviceResource {
public:
	CVideoPlayer();
	~CVideoPlayer();

	bool	play( const char* file );
	void	stop();
	bool	isPlaying();

	CD3DTexture& getTexture();
	int		getVideoX() const;
	int		getVideoY() const;
	int		getTextureX() const;
	int		getTextureY() const;

	// IDeviceResource
	virtual void createResource();
	virtual void activateResource();
	virtual void passivateResource();
	virtual void deleteResource();

private:
	IGraphBuilder*			mGraph;
	CDShowTextureRenderer*	mRenderer;
	IMediaControl*			mMediaControl;
	IMediaEvent*			mMediaEvent;
};
	
}; // namespace

#endif
