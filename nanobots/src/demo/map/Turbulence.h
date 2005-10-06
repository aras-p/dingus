#ifndef __TURBULENCE_H
#define __TURBULENCE_H

class CGameMap;


class CTurbulence : public boost::noncopyable {
public:
	struct SCell {
		float	vx;
		float	vy;
	};

public:
	CTurbulence( const CGameMap& gmap );
	~CTurbulence();

	void	update();

private:
	const CGameMap*	mMap;

	// turbulence cells array
	SCell*	mCells;
};


#endif
