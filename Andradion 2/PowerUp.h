// PowerUp.h: interface for the CPowerUp class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_POWERUP_H__B8CC0665_D7EF_11D2_B6FE_0050040B0541__INCLUDED_)
#define AFX_POWERUP_H__B8CC0665_D7EF_11D2_B6FE_0050040B0541__INCLUDED_

class CCharacter;
using NGameLib2::CGraphics;
using NGameLib2::CTimer;

class CPowerUp  
{
	friend bool NetRemoteLogic();

public:
	void Setup(FIXEDNUM x_,FIXEDNUM y_,const FIXEDNUM *ammo); // setup function used for ammo sets
	void Setup(FIXEDNUM x_,FIXEDNUM y_,int type_); // setup function used for normal powerups

	int Collides(const CCharacter& ch);
	void Draw(CGraphics& gr);
	void Logic();
	CPowerUp(const CPowerUp& r);
	CPowerUp& operator =(const CPowerUp& r);
	CPowerUp();
	~CPowerUp(); // defined in .cpp file

	static void Rotate(); // function keeps up with weapon powerup rotation
	__inline int      Type(               ) const { return this->type                        ;}
	__inline FIXEDNUM Ammo(int weapon_type) const { return this->ammo_contained[weapon_type] ;}

private:
	FIXEDNUM x; // coordinates are negative if the item has been picked up
	FIXEDNUM y;
	int type; // how to display it if it is an ammo set, otherwise, it defines what it is
	bool is_ammo_set;	

	union
	{
		FIXEDNUM *ammo_contained; // only used by ammo sets
		DWORD frames_since_picked_up; // not used by ammo sets
	};
	
	static DWORD rotation_timer;
	static int rotation_direction;
	static HANDLE beat_event;
	static int reference_count;
};

typedef std::vector<CPowerUp> VCTR_POWERUP;

#endif // !defined(AFX_POWERUP_H__B8CC0665_D7EF_11D2_B6FE_0050040B0541__INCLUDED_)
