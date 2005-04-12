// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __CONSOLE_H
#define __CONSOLE_H

#include "../utils/Singleton.h"

namespace dingus {

class CConsoleChannel;
class IConsoleRenderingContext;
class IConsoleFilter;


// --------------------------------------------------------------------------

class CConsole : public CSingleton<CConsole> {
public:
	static CConsoleChannel& CON_WARNING;
	static CConsoleChannel& CON_ERROR;

public:
	static CConsoleChannel& getChannel( const std::string& name ) { return getInstance().openChannel( name ); }

public:
	virtual ~CConsole();

	virtual CConsoleChannel& openChannel( const std::string& name );
	virtual void closeChannel( CConsoleChannel& channel );

	void setDefaultFilter( IConsoleFilter& filter ) { mDefaultFilter = &filter; };
	IConsoleFilter* getDefaultFilter() const { return mDefaultFilter; };		

	void setDefaultRenderingContext( IConsoleRenderingContext& ctx );
	IConsoleRenderingContext* getDefaultRenderingContext() const { return mDefaultRenderingCtx; };
	
private:
	IMPLEMENT_SIMPLE_SINGLETON(CConsole);
	CConsole();

protected:
	typedef std::list<CConsoleChannel*>	TChannelList;

private:
	IConsoleRenderingContext*	mDefaultRenderingCtx;
	IConsoleFilter*					mDefaultFilter;
	TChannelList				mChannels;

	static CConsole*			mSingleInstance;
};


// --------------------------------------------------------------------------

class CConsoleChannel {
public:
	CConsoleChannel( CConsole& console, const std::string& name, IConsoleRenderingContext* renderingCtx = NULL, IConsoleFilter* filter = NULL );
	virtual ~CConsoleChannel();

	virtual void close() { mParentConsole.closeChannel( *this ); }

	virtual void write( const std::string& methodName, const std::string& message );
	virtual void write( const std::string& message );

	void setRenderingContext( IConsoleRenderingContext& ctx ) { mRenderingCtx = &ctx; };
	IConsoleRenderingContext* getRenderingContext() const { return mRenderingCtx; };

	void setFilter( IConsoleFilter& filter ) { mFilter = &filter; };
	IConsoleFilter* getFilter() const { return mFilter; };

	const std::string& getName() const { return mName; };

	bool isWritingChannel() const { return mWritingChannel; }
	void setWritingChannel( bool wc ) { mWritingChannel = wc; }

	bool isLoggingTime() const { return mLoggingTime; }
	void setLoggingTime( bool wc ) { mLoggingTime = wc; }

	// output operators

	CConsoleChannel& endl() { write( buf); buf.erase(); return *this; };
	CConsoleChannel& operator <<( CConsoleChannel& (__cdecl * _f)(CConsoleChannel&) ){  (*_f)(*this); return *this; };
	CConsoleChannel& operator <<( const std::string& msg ) { buf += msg; return *this; }
	CConsoleChannel& operator <<( const char* msg ) { buf += msg; return *this; }
	CConsoleChannel& operator <<( int num ) { char msg[50]; itoa(num, msg, 10); buf += msg; return *this; }
	CConsoleChannel& operator <<( char c ) { buf += c; return *this; }
	CConsoleChannel& operator <<( DWORD num ) { char msg[50]; sprintf(msg,"%u",num); buf += msg; return *this; }
	CConsoleChannel& operator <<( float num ) { char msg[50]; sprintf(msg,"%g",num); buf += msg; return *this; }
	CConsoleChannel& operator <<( double num ) { char msg[50]; sprintf(msg,"%g",num); buf += msg; return *this; }
	CConsoleChannel& operator <<( D3DXCOLOR const& col ) { char msg[100]; sprintf( msg, "(%01.2f %01.2f %01.2f %01.2f)", col.r, col.g, col.b, col.a ); buf += msg; return *this; }
	CConsoleChannel& operator <<( D3DXVECTOR3 const& vec ) { char msg[100]; sprintf( msg, "(%01.2f %01.2f %01.2f)", vec.x, vec.y, vec.z ); buf += msg; return *this; }
	CConsoleChannel& operator <<( D3DXVECTOR4 const& vec ) { char msg[100]; sprintf( msg, "(%01.2f %01.2f %01.2f %01.2f)", vec.x, vec.y, vec.z, vec.w ); buf += msg; return *this; }
	CConsoleChannel& operator <<( D3DXQUATERNION const& q ) { char msg[100]; sprintf( msg, "(%01.2f %01.2f %01.2f %01.2f)", q.x, q.y, q.z, q.w ); buf += msg; return *this; }
	CConsoleChannel& operator <<( D3DXMATRIX const& m ) { char msg[200]; sprintf( msg,
		"(%01.2f %01.2f %01.2f %01.2f | %01.2f %01.2f %01.2f %01.2f | %01.2f %01.2f %01.2f %01.2f | %01.2f %01.2f %01.2f %01.2f)\n",
		m._11, m._12, m._13, m._14,
		m._21, m._22, m._23, m._24,
		m._31, m._32, m._33, m._34,
		m._41, m._42, m._43, m._44 ); buf += msg; return *this; }

protected:
	CConsole&		mParentConsole;
	std::string		buf;

private:
	std::string					mName;
	IConsoleRenderingContext*	mRenderingCtx;
	IConsoleFilter*					mFilter;
	bool						mWritingChannel;
	bool						mLoggingTime;
};

inline CConsoleChannel& __cdecl flush(CConsoleChannel& cc) { return cc.endl(); }
inline CConsoleChannel& __cdecl endl (CConsoleChannel& cc) { return cc.endl(); }


// --------------------------------------------------------------------------

class IConsoleRenderingContext {
public:
	virtual void write( const std::string& message ) = 0;
};


// --------------------------------------------------------------------------

class IConsoleFilter {
public:
	virtual bool apply( const CConsoleChannel& channel, const std::string& message ) const = 0;
};


// --------------------------------------------------------------------------

template<class _V, class _SEP = std::string>
class CConsoleOutputIterator {
public:
	typedef CConsoleOutputIterator<_V, _SEP>	this_type;

public:
	CConsoleOutputIterator( CConsoleChannel& chn, _SEP const& sep ) : mSeparator(sep), mChannel(chn) {}

	template<typename T>
		CConsoleOutputIterator const& operator=( T const& p ) { mChannel << p << mSeparator; return *this; }
	this_type& operator*() { return *this; }
	this_type& operator++() { return *this; }
	this_type& operator++( int ) { return *this; }

private:
	CConsoleChannel&	mChannel;
	_SEP const&	mSeparator;	
};


}; // namespace


#endif
