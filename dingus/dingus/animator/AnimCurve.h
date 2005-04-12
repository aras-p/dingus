// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __ANIM_CURVE_H
#define __ANIM_CURVE_H


namespace dingus {


/**
 *  Animation curve information.
 *
 *  This just has first value index (in array of values shared between several curves),
 *  interpolation type and a collapsed value (if curve is collapsed into single value).
 *
 *  @param _V Value type.
 */
template< typename _V >
class CAnimCurve {
public:
	typedef _V value_type;
	enum eIpol { NONE=0, STEP, LINEAR, CUBIC };

public:
	CAnimCurve( const value_type& collapsedVal, int firstSampleIndex, eIpol ipol )
		: mCollapsedValue(collapsedVal), mFirstSampleIndex(firstSampleIndex), mIpol(ipol) { }

	const value_type& getCollapsedValue() const { return mCollapsedValue; }
	int getFirstSampleIndex() const { return mFirstSampleIndex; }
	eIpol getIpol() const { return mIpol; }
	bool isCollapsed() const { return mIpol == NONE; }

private:
	value_type	mCollapsedValue;
	int			mFirstSampleIndex;
	eIpol		mIpol;
};


};


#endif
