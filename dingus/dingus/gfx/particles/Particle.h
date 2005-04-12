// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------

#ifndef __PARTICLE_H
#define __PARTICLE_H

#include "../../math/Vector3.h"


namespace dingus {


// --------------------------------------------------------------------------
//  simple particle

class CParticle {
public:
	CParticle( const SVector3& pos, const SVector3& vel, float lifetime )
		: mPosition(pos), mVelocity(vel), mInvLifetime(1.0f/lifetime), mTTL(lifetime) { }

	void update( float dt ) { mPosition += mVelocity*dt; mTTL -= dt; }

	float getInvLifetime() const { return mInvLifetime; }
	//void setTTL( float ttl ) { mTTL = ttl; }
	float getTTL() const { return mTTL; }
	bool isAlive() const { return mTTL > 0; }

	SVector3 const& getPosition() const { return mPosition; }
	SVector3& getPosition() { return mPosition; }
	SVector3 const& getVelocity() const { return mVelocity; }
	SVector3& getVelocity() { return mVelocity; }

private:
	SVector3	mPosition;
	SVector3	mVelocity;
	float		mTTL;
	float		mInvLifetime;
};


// --------------------------------------------------------------------------
//  colored particle

class CColoredParticle : public CParticle {
public:
	struct SState {
		SState( D3DXCOLOR col )
			: color(col) { }
		D3DXCOLOR	color;
	};
public:
	CColoredParticle( const SVector3& pos, const SVector3& vel, D3DCOLOR color, float lifetime )
		: CParticle(pos,vel,lifetime), mColor(color) { }

	D3DCOLOR getColor() const { return mColor; }
	void setColor( D3DCOLOR color ) { mColor = color; }

	void setFromState( const SState& a, const SState& b, float t ) {
		D3DXCOLOR c;
		D3DXColorLerp( &c, &a.color, &b.color, t );
		mColor = c;
	};

private:
	D3DCOLOR	mColor;
};


// --------------------------------------------------------------------------
//  sized, rotated and colored particle

class CSizedRotColoredParticle : public CColoredParticle {
public:
	struct SState : public CColoredParticle::SState {
		SState( D3DXCOLOR col, float siz, float rot )
			: CColoredParticle::SState(col), size(siz), rotation(rot) { }
		float	size;
		float	rotation;
	};
public:
	CSizedRotColoredParticle( const SVector3& pos, const SVector3& vel, D3DCOLOR color, float size, float rot, float lifetime )
		: CColoredParticle(pos,vel,color,lifetime), mOrigSize(size), mSizeFraction(1.0f), mOrigRotation(rot), mRotation(rot) { }

	float getOrigSize() const { return mOrigSize; }

	float getSizeFraction() const { return mSizeFraction; }
	void setSizeFraction( float s ) { mSizeFraction = s; }

	float getRotation() const { return mRotation; }
	void setRotation( float r ) { mOrigRotation = mRotation = r; }

	void setFromState( const SState& a, const SState& b, float t ) {
		CColoredParticle::setFromState( a, b, t );
		mSizeFraction = a.size + (b.size - a.size) * t;
		mRotation = mOrigRotation + a.rotation + (b.rotation - a.rotation) * t;
	};

private:
	float		mOrigSize, mSizeFraction;
	float		mOrigRotation, mRotation;
};


// --------------------------------------------------------------------------
//  animated, sized, rotated and colored particle

class CAnimatedSizedRotColoredParticle : public CSizedRotColoredParticle {
public:
	CAnimatedSizedRotColoredParticle( const SVector3& pos, const SVector3& vel, D3DCOLOR color, float size, float rot, float lifetime )
		: CSizedRotColoredParticle( pos, vel, color, size, rot, lifetime ) 
	{}

	/*void setFromState( const SState& a, const SState& b, float t ) {
		CColoredParticle::setFromState( a, b, t );
		mSizeFraction = a.size + (b.size - a.size) * t;
		mRotation = mOrigRotation + a.rotation + (b.rotation - a.rotation) * t;
	};*/

	void setTextureUV( float tu, float tv ) {
		mTu = tu;
		mTv = tv;
	}
	float getTextureU() const { return mTu; }
	float getTextureV() const { return mTv; }

private:
	float	mTu, mTv;
};


// --------------------------------------------------------------------------
//  animated blended, sized, rotated and colored particle

class CAnimatedBlendedSizedRotColoredParticle : public CSizedRotColoredParticle {
public:
	CAnimatedBlendedSizedRotColoredParticle( const SVector3& pos, const SVector3& vel, D3DCOLOR color, float size, float rot, float lifetime )
		: CSizedRotColoredParticle( pos, vel, color, size, rot, lifetime ) 
	{}

	/*void setFromState( const SState& a, const SState& b, float t ) {
		CColoredParticle::setFromState( a, b, t );
		mSizeFraction = a.size + (b.size - a.size) * t;
		mRotation = mOrigRotation + a.rotation + (b.rotation - a.rotation) * t;
	};*/

	void setTextureUV1( float tu1, float tv1 ) { mTu1 = tu1; mTv1 = tv1; }
	float getTextureU1() const { return mTu1; }
	float getTextureV1() const { return mTv1; }

	void setTextureUV2( float tu2, float tv2 ) { mTu2 = tu2; mTv2 = tv2; }
	float getTextureU2() const { return mTu2; }
	float getTextureV2() const { return mTv2; }

	void setColor1Factor( float f ) { mColor1 = getColor(); mColor1 = ( int( ( mColor1 >> 24 ) * f ) << 24 ) | ( mColor1 & 0x00ffffff ); }
	//void setColor1Factor( float f ) { mColor1 = getColor(); mColor1 = ( int( 0xff * f ) << 24 ) | ( mColor1 & 0x00ffffff ); }
	D3DCOLOR getColor1() const { return mColor1; }
	void setColor2Factor( float f ) { mColor2 = getColor(); mColor2 = ( int( ( mColor2 >> 24 ) * f ) << 24 ) | ( mColor2 & 0x00ffffff ); }
	//void setColor2Factor( float f ) { mColor2 = getColor(); mColor2 = ( int( 0xff * f ) << 24 ) | ( mColor2 & 0x00ffffff ); }
	D3DCOLOR getColor2() const { return mColor2; }
	
private:
	float	mTu1, mTv1;
	float	mTu2, mTv2;
	
	D3DCOLOR mColor1;
	D3DCOLOR mColor2;
};



}; // namespace

#endif
