#ifndef __GAME_TYPES_H
#define __GAME_TYPES_H

enum {
	G_MAX_PLAYER_COUNT = 3,	///< 2 players and AI
}; 


/// Entity types.
enum eEntityType {
	ENTITY_NEEDLE = 0,
	ENTITY_EXPLORER,
	ENTITY_COLLECTOR,
	ENTITY_AI,
	ENTITY_CUREBOT,
	ENTITY_PILOT,
	ENTITY_BLOCKER,
	ENTITY_CELL_W,
	ENTITY_CELL_B,
	ENTITYCOUNT
};

/// Entity states.
enum eEntityState {
	ENTSTATE_IDLE = 0,
	ENTSTATE_MOVE,
	ENTSTATE_ATTACK,
	ENTSTATE_COLLECT,
	ENTSTATE_TRANSFER,
	ENTSTATE_BUILD,
	ENTSTATECOUNT
};

/// Cell types
enum eCellType {
	CELL_BLOOD1 = 0,	///< Low density blood
	CELL_BLOOD2,		///< Medium density blood
	CELL_BLOOD3,		///< High density blood
	CELL_PERF,			///< Perforable cell
	CELL_BONE,			///< Bone
	CELLCOUNT
};

/// Special point types
enum ePointType {
	PT_AZN = 0,
	PT_HOSHIMI,
	PT_INJECTION,	// Is not in the maps
	PTCOUNT
};


#endif
