// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __FX_PARAMS_NOTIFIER_H
#define __FX_PARAMS_NOTIFIER_H

#include "EffectParams.h"
#include "../utils/Notifier.h"

namespace dingus {

/**
 *  Notifies effect params on effect object change.
 *
 *  Effect params objects register to the notifier, and unregister when
 *  destroyed. You need to notify them when the effect objects change (eg. on
 *  effects reload, on device change etc.).
 */
class CEffectParamsNotifier : public CNotifier<CEffectParams> {
public:
	static CEffectParamsNotifier& getInstance() { return mSingleInstance; }

	void notify();

private:
	CEffectParamsNotifier() { }
	static CEffectParamsNotifier mSingleInstance;
};

}; // namespace

#endif
