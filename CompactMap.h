class SurfaceFiller;

class CCompactMap {
  Buffer left_over_pixels, pattern_pixels;

  typedef struct {
    BYTE x, opaque_count;
  } RUN;

  typedef std::vector<RUN> ROW;
  typedef std::vector<ROW> VCTR_ROW;

  VCTR_ROW left_over;

  typedef struct {
    BYTE x, y, w, h;
  } RCT;
  
  /** Colors of the blocks. */
  std::vector<BYTE> blocks;

  /** Coordinates of the blocks. */
  std::vector< std::vector<RCT> > block_areas;

  typedef struct {
    BYTE x, y;
  } PNT;

  typedef struct {
    std::vector<PNT> spots;
    BYTE width, height;
  } PATTERN;

  std::vector<PATTERN> patterns;

  // this function needs the surface to be locked
  // clipping is not supported
  void RenderStep2(void *surface, int pitch);

  friend class CmpFiller;

public:
  CCompactMap(BYTE **source);
  
  static std::auto_ptr<std::vector<CCompactMap *> >
  LoadMapSet(const char *res_name, const char *type,
             HMODULE module, WORD language);             

  // uses manual clipper and BltFast.
  // pass: target surface, target x and y, and clipper rectangle
  void RenderStep1(IDirectDrawSurface *, int, int, const RECT&);

  inline bool NoStep2() const {return left_over.empty()
                                 && patterns.empty();}

  std::auto_ptr<SurfaceFiller> Filler();
};
