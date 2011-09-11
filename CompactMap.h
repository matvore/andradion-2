class CompactMap {
  Buffer left_over_pixels, pattern_pixels;

  typedef struct {
    BYTE x, opaque_count;
  } RUN;

  typedef Array<RUN> ROW;
  typedef Array<ROW> VCTR_ROW;

  VCTR_ROW left_over;

  typedef struct {
    BYTE x, y, w, h;
  } RCT;
  
  /** Colors of the blocks. */
  Array<BYTE> blocks;

  /** Coordinates of the blocks. */
  Array<Array<RCT> > block_areas;

  typedef struct {
    BYTE x, y;
  } PNT;

  typedef struct {
    Array<PNT> spots;
    BYTE width, height;
  } PATTERN;

  Array<PATTERN> patterns;

  /** Renders this compact map to the given locked surface region. No clipping
   * is performed.
   */
  void Render(BYTE *surf_ptr, int pitch);

  friend class CmpFiller;

public:
  CompactMap(BYTE **source);
  
  static std::auto_ptr<std::vector<CompactMap *> >
  LoadMapSet(const char *type, const char *res_name);

  /** Renders the "step 1" component of the compact map, which is comprised
   * entirely of rectangles. A lock must be obtained on the buffer of the given
   * Graphics object, which is done automatically if there is no lock already.
   *
   * @param gfx the Graphics object to which to draw the rectangles.
   * @param x the x-coordinate at which to draw the rectangles.
   * @param y the y-coordinate at which to draw the rectangles.
   * @param clip Indicates whether to clip the rectangles to the Graphics
   *  buffer. If this is false when portions of the rectangles reach outside of
   *  the Graphics buffer, the results are undefined.
   */
  void RenderStep1(Gfx *gfx, int x, int y, bool clip);

  inline bool NoStep2() {return left_over.Empty() && patterns.Empty();}

  std::auto_ptr<Gfx::SurfaceFiller> Filler();
};
