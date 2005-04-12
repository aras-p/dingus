// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __REGISTRY_H
#define __REGISTRY_H

namespace dingus {


template<typename KEY, typename VALUE>
class CRegistry {
public:
	typedef std::map<KEY,VALUE>	TKeyValueMap;
public:
	bool isValue( const KEY& key ) const;
	const VALUE& getValue( const KEY& key ) const;
	VALUE& getValue( const KEY& key );
	const VALUE* findValue( const KEY& key ) const;
	VALUE* findValue( const KEY& key );
	void setValue( const KEY& key, const VALUE& value );
	void clearValue( const KEY& key );

	const TKeyValueMap& getValues() const { return mRegistry; }
	TKeyValueMap& getValues() { return mRegistry; }
	
private:
	TKeyValueMap	mRegistry;
};


template<typename KEY, typename VALUE>
bool CRegistry<KEY,VALUE>::isValue( const KEY& key ) const
{
	TKeyValueMap::const_iterator it = mRegistry.find( key );
	return it != mRegistry.end();
}

template<typename KEY, typename VALUE>
const VALUE& CRegistry<KEY,VALUE>::getValue( const KEY& key ) const
{
	TKeyValueMap::const_iterator it = mRegistry.find( key );
	assert( it != mRegistry.end() );
	return it->second;
}

template<typename KEY, typename VALUE>
VALUE& CRegistry<KEY,VALUE>::getValue( const KEY& key )
{
	TKeyValueMap::iterator it = mRegistry.find( key );
	assert( it != mRegistry.end() );
	return it->second;
}

template<typename KEY, typename VALUE>
const VALUE* CRegistry<KEY,VALUE>::findValue( const KEY& key ) const
{
	TKeyValueMap::const_iterator it = mRegistry.find( key );
	return (it == mRegistry.end()) ? 0 : (&it->second);
}

template<typename KEY, typename VALUE>
VALUE* CRegistry<KEY,VALUE>::findValue( const KEY& key )
{
	TKeyValueMap::iterator it = mRegistry.find( key );
	return (it == mRegistry.end()) ? 0 : (&it->second);
}

template<typename KEY, typename VALUE>
void CRegistry<KEY,VALUE>::setValue( const KEY& key, const VALUE& value )
{
	assert( mRegistry.find(key) == mRegistry.end() );
	mRegistry.insert( std::make_pair( key, value ) );
}

template<typename KEY, typename VALUE>
void CRegistry<KEY,VALUE>::clearValue( const KEY& key )
{
	mRegistry.erase( key );
}

}; // namespace

#endif
