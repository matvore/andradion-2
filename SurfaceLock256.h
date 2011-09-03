class CGraphics;

class CSurfaceLock256 : public CSurfaceLock {
 public:
  CSurfaceLock256(); // this constructor does nothing, just initiates so it is ready to be setup eventually
  CSurfaceLock256(LPDIRECTDRAWSURFACE2 target_surface_,RECT *area_ = NULL,DWORD lock_flags_ = DDLOCK_WAIT);
  virtual void Line(int x0,int y0,int x1,int y1,DWORD color,DWORD pattern = 0xffffffff);
};
