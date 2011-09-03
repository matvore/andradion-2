using std::vector;

const int MAX_FIRES = 10; // maximum number of bullets at a time

class CFire {
 public:
  bool OkayToDelete() const;
  
  // pass zero for the last parameter when the gun was fired by a
  // remote player 
  void Setup(FIXEDNUM sx_, FIXEDNUM sy_,
             int direction_, int type_,
             bool remotely_generated_);

  // use for bazookas triggered by remote systems
  void Setup(FIXEDNUM sx_, FIXEDNUM sy_,
             FIXEDNUM tx_, FIXEDNUM ty_);
  
  void Logic();
  void Draw();
  static void PickBestBulletTrailColor();

  CFire();

 private:
  class bullet_trail_line_color {
  public:
    BYTE operator()() {return CFire::bullet_trail_color;}
  };

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

extern CFire fires[MAX_FIRES];
