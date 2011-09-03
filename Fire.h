using std::vector;

class CFire {
 public:
  inline bool OkayToDelete() const {return FIRESTATE_INACTIVE == this->state;}
  // pass zero for the last parameter when the gun was fired by a remote player
  void Setup(FIXEDNUM sx_,FIXEDNUM sy_,int direction_,int type_,bool remotely_generated_);
  void Setup(FIXEDNUM sx_,FIXEDNUM sy_,FIXEDNUM tx_,FIXEDNUM ty_); // use for bazookas triggered by remote systems
  void Logic();
  void Draw(CGraphics& gr);
  static void PickBestBulletTrailColor();

  CFire();

 private:
  void PlaySound();
  static BYTE bullet_trail_color;
  vector<int> CollidesEx();
  int Collides();
  FIXEDNUM x, y;
  FIXEDNUM sx, sy; // start location
  int direction, type, state, horizontal_collision_flags;

  DWORD frames_since_explosion_started;

  bool remotely_generated;

  // when a bazooka is remotely generated, the start and stop
  //  coordinates have already been determined by a remote computer
};
