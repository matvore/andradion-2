enum {POWERUP_PISTOLAMMO, POWERUP_MACHINEGUNAMMO,
      POWERUP_BAZOOKAAMMO, POWERUP_HEALTHPACK};
enum {POWERUPCOLLIDES_NOTHINGHAPPENED,
      POWERUPCOLLIDES_PICKEDUPANDWILLNOTREGENERATE,
      POWERUPCOLLIDES_PICKEDUPANDWILLREGENERATE};

class CCharacter;

class CPowerUp {
  friend void NetRemoteLogic();

 public:
  // setup function used for ammo sets
  void Setup(FIXEDNUM x_, FIXEDNUM y_, const FIXEDNUM *ammo);
  
  // setup function used for normal powerups
  void Setup(FIXEDNUM x_, FIXEDNUM y_, unsigned int type_); 

  int Collides(const CCharacter& ch);
  void Draw();
  void Logic();
  CPowerUp(const CPowerUp& r);
  CPowerUp& operator =(const CPowerUp& r);
  CPowerUp();
  ~CPowerUp();

  static void Rotate(); // function keeps up with weapon powerup rotation

  inline bool Regenerates() const {
    return type >= 0;
  }
  
  inline int Type() const {return type >= 0 ? type : ~type;}

  inline void PickUp() {
    assert((y > 0) == (x > 0));
    if (x > 0) {
      x *= -1;
      y *= -1;
    }
    frames_since_picked_up = 0;
  }
  
  inline FIXEDNUM Ammo(unsigned int weapon_type) const {
    assert(type < 0);
    return ammo_contained[weapon_type];
  }

 private:
  // coordinates are negative if the item has been picked up
  FIXEDNUM x, y;

  // how to display it if it is an ammo set,
  //  otherwise, it defines what it is
  int type; 

  union {
    FIXEDNUM *ammo_contained; // only used by ammo sets
    DWORD frames_since_picked_up; // not used by ammo sets
  };

  static DWORD rotation_timer;
  static int rotation_direction;
  static HANDLE beat_event;
  static unsigned int reference_count;
};

typedef std::vector<CPowerUp> VCTR_POWERUP;
