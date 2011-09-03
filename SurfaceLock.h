class CGraphics;
class CBob;

const int SURFACELOCKERR_OUTOFMEMORY = 1;
const int SURFACELOCKERR_WASSTILLDRAWING = 2;
const int SURFACELOCKERR_SURFACELOST = 3;

class CSurfaceLock : public CCertifiable {
  friend class CGraphics;

 public:
  CSurfaceLock(); // this constructor does nothing, just initiates so it is ready to be setup eventually
  CSurfaceLock(LPDIRECTDRAWSURFACE2 target_surface_,RECT *area_ = NULL,DWORD lock_flags_ = DDLOCK_WAIT);

  virtual int Certify(); // certify means lock
  virtual void Uncertify(); // uncertify means unlock

  void SetTargetSurface(CGraphics& target_display_); // an additional way to set the target surface
  void SetTargetSurface(CBob& target_bob_); // an additional way to set the target surface

  // draws a line (use the ClipLine function of CGraphics to clip the line)
  virtual void Line(int x0,int y0,int x1,int y1,DWORD color,DWORD pattern = 0xffffffff) = 0;

  // get the surface description of the surface/lock.  Use it to do your own kind of drawing
  //  whatever that may be, now that you have a pointer to the surface memory.  The function
  //  is defined later in the header file
  inline const DDSURFACEDESC& SurfaceDesc() const;
		
 protected: DDSURFACEDESC ddsd;
 protected: CertParamA(RECT *,area,Area);
 protected: CertParamA(DWORD,lock_flags,LockBehaviorFlags);
 protected: CertParamA(IDirectDrawSurface2 *,target_surface,TargetSurface);
};
	
inline const DDSURFACEDESC& CSurfaceLock::SurfaceDesc() const {
  assert(this->Certified());
  return this->ddsd;
}
