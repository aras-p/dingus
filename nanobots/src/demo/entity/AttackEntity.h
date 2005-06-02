#ifndef __ATTACK_ENTITY_H
#define __ATTACK_ENTITY_H

class CAttack;
class CExplosion;
class CNimbus;


class CAttackEntityManager : public boost::noncopyable {
public:
	CAttackEntityManager();
	~CAttackEntityManager();

	void	begin();
	void	renderAttack( const SVector3& origin, int targx, int targy, float turnAlpha, const SVector4& color );
	void	renderExplosion( const SVector3& origin, float turnAlpha );
	void	renderNimbus( const SVector3& origin, float scale );
	void	end() { };

private:
	std::vector<CAttack*>		mAttacks;
	std::vector<CExplosion*>	mExplosions;
	std::vector<CNimbus*>		mNimbi;
	int		mAttackCounter;
	int		mExplosionCounter;
	int		mNimbusCounter;
};



#endif
