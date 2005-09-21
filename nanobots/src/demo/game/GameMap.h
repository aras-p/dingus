#ifndef __GAME_MAP_H
#define __GAME_MAP_H


/// Wall-bounding cells have this height. The further from the walls, the higher height.
const float MIN_CELL_HEIGHT = 1.5f;


class CGameMap : public boost::noncopyable {
public:
	struct SPoint {
		SPoint( ePointType atype, int ax, int ay, int d = 0 );
		ePointType	type;
		int			x;
		int			y;
		int			data;
		D3DCOLOR	colorMain;
		D3DCOLOR	colorTone;
	};
	
	struct SCell {
		float		height;	// half of floor-ceiling distance
		eCellType	type;
		bool		nearBone; // some of 8-neighbour cells is bone
	};

public:
	CGameMap();
	~CGameMap();
	
	/// @return Empty string if ok, error message on error.
	std::string	initialize();

	bool	checkMapValidity() const;

	static bool isBlood( int type ) { return type < CELL_BONE; }

	int		getCellsX() const { return mCellsX; }
	int		getCellsY() const { return mCellsY; }
	const SCell& getCell( int index ) const { return mCells[index]; }
	const SCell& getCell( int x, int y ) const { assert(x>=0&&x<mCellsX&&y>=0&&y<mCellsY); return mCells[y*mCellsX+x]; }
	const SCell* getCells() const { return mCells; }
	int		pos2index( int x, int y ) const { assert(x>=0&&x<mCellsX&&y>=0&&y<mCellsY); return y*mCellsX+x; }

	bool	isCellBlood( int index ) const { return isBlood(mCells[index].type); }

	int		getPointCount() const { return mPoints.size(); }
	const SPoint& getPoint( int i ) const { return mPoints[i]; }
	void	addInjectionPoint( int player, int x, int y ) { mPoints.push_back( SPoint(PT_INJECTION,x,y,player) ); }
	void	addObjectivePoint( int mission, int x, int y ) { mPoints.push_back( SPoint(PT_OBJECTIVE,x,y,mission) ); }

	const std::string& getName() const { return mName; }
	unsigned int getCRC() const { return mCRC; }

private:
	void	calcCellHeights();
	
private:
	// Map
	int		mCellsX;
	int		mCellsY;
	SCell*	mCells;

	// Special points
	std::vector<SPoint>	mPoints;

	std::string		mName;	/// Map's name
	unsigned int	mCRC;	/// CRC 32 of the cells
};


#endif
