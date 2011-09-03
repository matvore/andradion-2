// Fire.h: interface for the CFire class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FIRE_H__B8CC0663_D7EF_11D2_B6FE_0050040B0541__INCLUDED_)
#define AFX_FIRE_H__B8CC0663_D7EF_11D2_B6FE_0050040B0541__INCLUDED_

using NGameLib2::CGraphics;

class CFire  
{
public:
  inline bool OkayToDelete() const {return FIRESTATE_INACTIVE == this->state;}
  // pass zero for the last parameter when the gun was fired by a remote player
  void Setup(FIXEDNUM sx_,
	     FIXEDNUM sy_,
	     int direction_,
	     int type_,
	     bool remotely_generated_);
  void Setup(FIXEDNUM sx_,
	     FIXEDNUM sy_,
	     FIXEDNUM tx_,
	     FIXEDNUM ty_); // use for bazookas triggered by remote
			    // systems  
  void Logic();
  void Draw(CGraphics& gr);
  static void PickBestBulletTrailColor();

  CFire();

private:
  void PlaySound();
  static BYTE bullet_trail_color;
  vector<int> CollidesEx();
  int Collides();
  FIXEDNUM x;
  FIXEDNUM y;
  FIXEDNUM sx; // start location
  FIXEDNUM sy;
  int direction;
  int type;

  int state;

  int horizontal_collision_flags;

  DWORD frames_since_explosion_started;

  bool remotely_generated;

  // when a bazooka is remotely generated, the start and stop
  //  coordinates have already been determined by a remote computer
};

#endif // !defined(AFX_FIRE_H__B8CC0663_D7EF_11D2_B6FE_0050040B0541__INCLUDED_)
