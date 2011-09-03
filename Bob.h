const DWORD SCAPS_NONE = (DWORD)+0;
const DWORD SCAPS_ALL  = (DWORD)-1;

class CColorNP;

class CBob {
  friend class CGraphics;
  friend class CSurfaceLock;

 public:
  int GetWidth();
  int GetHeight();
  void GetSize(int& width,int& height);
  void Create(int w,int h,DWORD orFlags=SCAPS_NONE,DWORD andFlags=SCAPS_ALL);
  void Extract(HBITMAP bmp,int x,int y);
  void SetTransparentColor(BYTE c); // use for 256-color bmps
  void SetTransparentColor(const CColorNP& c); // use for non-palettized bmps
  CBob(int w,int h,DWORD orFlags=SCAPS_NONE,DWORD andFlags=SCAPS_ALL);
  CBob(HBITMAP bmp,DWORD orFlags=SCAPS_NONE,DWORD andFlags=SCAPS_ALL);
  CBob(LPDIRECTDRAWSURFACE2 parent);
  CBob();
  virtual ~CBob();
  inline LPDIRECTDRAWSURFACE2 Data() {return this->data;}
  bool IsCreated() const;
  void Create(LPDIRECTDRAWSURFACE2 parent);
  void Destroy();

 protected:
  void SetTransparentColorInternal(DWORD c);
  LPDIRECTDRAWSURFACE2 data;
  RECT entire;
};
