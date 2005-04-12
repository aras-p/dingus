// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __RESOURCE_ID_H
#define __RESOURCE_ID_H

namespace dingus {

/**
 *  Resource identifier.
 *
 *  Resources are identified by CResourceId, which is essentially
 *  a read-only std::string, refering to a file in most cases.
 */
class CResourceId {
public:
	CResourceId( const char *id );
	CResourceId( const std::string& id );
	CResourceId( const CResourceId& rid );

	const std::string& getUniqueName() const;

	const CResourceId& operator=( const CResourceId& rhs );
	
	bool operator== ( const CResourceId& rhs ) const;
	bool operator!= ( const CResourceId& rhs ) const;
	bool operator< ( const CResourceId& rhs ) const;
	bool operator> ( const CResourceId& rhs ) const;
	
private:
	std::string	mId;
};



// ------------------------------------------------------------------
//  implementation

inline CResourceId::CResourceId( const char* id )
:	mId( id )
{
}
inline CResourceId::CResourceId( const std::string& id )
:	mId( id )
{
}
inline CResourceId::CResourceId( const CResourceId& rid )
:	mId( rid.mId )
{
}
inline const std::string& CResourceId::getUniqueName() const
{
	return mId;
};
inline const CResourceId& CResourceId::operator=( const CResourceId& rhs )
{
	if( this == &rhs )
		return *this;
	mId = rhs.mId;
	return *this;
}
inline bool CResourceId::operator== ( const CResourceId& rhs ) const
{
	if( &rhs == this )
		return true;
	return ( mId == rhs.getUniqueName() );
}
inline bool CResourceId::operator!= ( const CResourceId& rhs ) const
{
	return !( CResourceId::operator==( rhs ) );
}
inline bool CResourceId::operator< ( const CResourceId& rhs ) const
{
	return ( mId < rhs.getUniqueName() );
}
inline bool CResourceId::operator> ( const CResourceId& rhs ) const
{
	return ( mId > rhs.getUniqueName() );
}



}; // namespace

#endif
