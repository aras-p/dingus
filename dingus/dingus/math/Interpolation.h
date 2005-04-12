// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __MATH_INTERPOLATE_H_
#define __MATH_INTERPOLATE_H_

namespace dingus {


/**
 *  Linear interpolates between 2 values.
 *  @param _R Result type
 *  @param _A Value type
 *  @param _T Time type
 */
template<typename _R, typename _A, typename _T>
inline _R const math_lerp( _A const& a1, _A const& a2, _T const& t )
{
	return _R( a1 + (a2-a1)*t );
};


/**
 *  Performs Catmull-Rom interpolation between 4 values.
 *  @param _R Result type
 *  @param _A Value type
 *  @param _T Time type
 */
template<typename _R, typename _A, typename _T>
_R const math_catmull_rom( _A const& a0, _A const& a1, _A const& a2, _A const& a3, _T const& t )
{
	_T tt = t*t;
	_T ttt = t*tt;
	return _R( (a0 * ( -t     + tt+tt   - ttt      ) +
				a1 * ( 2.0f   - 5.0f*tt + ttt*3.0f ) +
				a2 * (  t     + 4.0f*tt - ttt*3.0f ) +
				a3 * (        - tt      + ttt      ) ) * 0.5f );
};



}; // namespace

#endif
