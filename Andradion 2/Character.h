// Character.h: interface for the CCharacter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHARACTER_H__B8CC0661_D7EF_11D2_B6FE_0050040B0541__INCLUDED_)
#define AFX_CHARACTER_H__B8CC0661_D7EF_11D2_B6FE_0050040B0541__INCLUDED_

using NGameLib2::CGraphics;
using NGameLib2::CBob;
using std::pair;

class CCharacter;

struct TCharacterPointer
{
  TCharacterPointer(CCharacter *p) : ch(p) {}
  CCharacter *ch;
};

class CCharacter  
{
  friend BOOL FAR PASCAL EnumPlayersCB(
				       DPID dpId,DWORD dwPlayerType,LPCDPNAME lpName,DWORD dwFlags,LPVOID lpContext
				       );
  friend bool NetRemoteLogic();
  friend bool operator >(const TCharacterPointer&,const TCharacterPointer&);
  friend bool operator <(const TCharacterPointer&,const TCharacterPointer&);

 public:
  // this calculate sector function figures out what sector the character
  //  should be in and sets its members accordingly, setting the row and col
  //  parameters to match
  void CalculateSector(int& row,int& col);
  bool IsOffScreen() const;
  void Logic(const CCharacter& target);
	
  void Logic();
  void Setup(FIXEDNUM x,FIXEDNUM y,int model_,bool doing_mp); // used for locals (enemies or hero)
  bool DrawCharacter(CGraphics& gr); // returns true if character has moved
  void DrawMeters(CGraphics& gr,int show_health);
  void SubtractHealth(int fire_type);
  void TryToFire();
  void CheckForEnemyCollision(const CCharacter&);

  // simple accessors:
  inline void GetLocation (FIXEDNUM& x,FIXEDNUM& y) const {x = this->coor.first.x; y = this->coor.first.y - Fixed(TILE_HEIGHT/2) ;}
  inline FIXEDNUM X     () const { return this->coor.first.x                             ;}
  inline FIXEDNUM Y     () const { return this->coor.first.y - Fixed(TILE_HEIGHT/2) ;}
  inline int   Model () const { return this->model                                    ;}
  inline bool  Dead  () const { return CHARSTATE_DEAD == this->state || CHARSTATE_DYING == this->state ;}

  inline void GetSector(int& row,int& col) const { row = this->sector_row; col = this->sector_col ;}

  inline bool HasFullHealth (               ) const { return Fixed(1) == this->health            ;}
  inline bool HasFullAmmo   (int weapon_type) const { return Fixed(1) == this->ammo[weapon_type] ;}
  inline FIXEDNUM Health() const {return this->health;}

 private:
  void ResetAmmo(); // function that resets ammo to original count (some pistol ammo, no machine gun or bazooka)
  void TryToMove();
  void PlaySound();
  void PowerUpCollisions();

  // this function blits the specified object to the screen using the
  // current target rectangle as the origin, shaking the object
  // according to weapon recoil
  void BlitterRectRecoil(CGraphics&,CBob&);

  // these members are utilized for
  //  all kinds of characters
  int current_weapon;
  int model;
  int direction;
  pair<POINT,POINT> coor;
  int state;
  bool reset_gamma; // used by draw function

  DWORD frames_since_last_fire;
  DWORD frames_in_this_state;
  DWORD frames_not_having_sworn;

  FIXEDNUM ammo[3];
  FIXEDNUM health;

  int sector_row;
  int sector_col;
};

// these less-than, greater-than operators help with sorting in the
//  drawing order
inline bool operator >(const TCharacterPointer& l,const TCharacterPointer& r)
{
  return l.ch->coor.first.y > r.ch->coor.first.y || true == r.ch->Dead();
}

inline bool operator <(const TCharacterPointer& l,const TCharacterPointer& r)
{
  return l.ch->coor.first.y < r.ch->coor.first.y || true == l.ch->Dead();
}

typedef std::vector<CCharacter> VCTR_CHARACTER;
#endif // !defined(AFX_CHARACTER_H__B8CC0661_D7EF_11D2_B6FE_0050040B0541__INCLUDED_)
