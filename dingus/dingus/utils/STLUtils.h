// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __STL_UTILS_H__
#define __STL_UTILS_H__

namespace dingus {

namespace stl_utils {

/** Deletes items in pointer container. */
template<class T>
void wipe( T& cont )
{
	for( T::iterator it = cont.begin(); it != cont.end(); ++it )
		delete *it;
	cont.clear();
};

/** Deletes values in map (values by pointers). */
template<class T>
void wipe_map( T& cont )
{
	for( T::iterator it = cont.begin(); it != cont.end(); ++it )
		delete it->second;
	cont.clear();
};

/** Same as "for_each", but for the whole container at once. */
template<typename _T, typename _O>
void for_all( _T& cont, _O& op )
{
	for( _T::iterator it = cont.begin(); it != cont.end(); ++it )
		op( *it );
};

/** Same as "for_each", but for the whole container at once. */
template<typename _T, typename _O>
void for_all( _T const& cont, _O& op )
{
	for( _T::const_iterator it = cont.begin(); it != cont.end(); ++it )
		op( *it );
};

}; // namespace stl_utils


}; // namespace

#endif 
