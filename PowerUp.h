class CCharacter;

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
  ~CPowerUp();

  static void Rotate(); // function keeps up with weapon powerup rotation
  inline int      Type(               ) const {return type >= 0 ? type : -1-type;}
  inline FIXEDNUM Ammo(int weapon_type) const
    {
      assert(type < 0);
      return this->ammo_contained[weapon_type];
    }

 private:
  FIXEDNUM x, y; // coordinates are negative if the item has been picked up
  int type; // how to display it if it is an ammo set, otherwise, it defines what it is

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
