struct IDirectDrawClipper;

class GfxPretty : public Gfx {
public:
  class SurfacePretty : public Surface {
  public:
    SurfacePretty(GfxPretty *owner, int width, int height)
      : Surface(owner, width, height,
                owner->CreateOffscreenSurface(width, height * 2)) {}

    virtual void Draw(int x, int y, bool transparency);
    virtual void DrawScale(const RECT *dest_rect, const RECT *source_rect,
                           bool transparency);

    virtual bool Refill();
  };

  class FontPretty : public Font {
    friend class GfxPretty;

  public:
    FontPretty(GfxPretty *owner, void *resource_data, int font_width,
               int font_height, int first_font_char, int last_font_char)
      : Font(owner, resource_data, font_width, font_height,
             first_font_char, last_font_char) {}

    virtual void WriteChar(int x, int y, int ch, BYTE color);
  };

protected:
  /** Indicates the "dimmer" version of each color. */
  BYTE dim_palette[256];

  AutoComPtr<IDirectDrawClipper> clipper;

  RECT physical_clip_rect;

  void WriteToFrontBuffer(FontPretty *font, BYTE *surf, int pitch,
                          int ch, BYTE color, BYTE back_color);

  static GfxPretty *gfx_pretty;
  GfxPretty(HWND hWnd);

public:
  virtual ~GfxPretty() {gfx_pretty = 0;}

  static inline GfxPretty *Get() {return gfx_pretty;}
  static void Initialize(HWND hWnd) {
    gfx_pretty = new GfxPretty(hWnd);
    gfx.reset(gfx_pretty);
  }

  virtual std::auto_ptr<Font>
  LoadFont(HGLOBAL resource_handle, int font_width, int font_height,
           int first_font_char, int last_font_char);

  virtual void Flip();

  virtual void Lock();

  virtual void Rectangle(const RECT *area, BYTE color, bool clip);

  virtual void SetPalette(PALETTEENTRY *pe, int entry_count,
                          bool visual_effect);

  virtual std::auto_ptr<Surface> CreateSurface
  (int w, int h, std::auto_ptr<SurfaceFiller> filler);

  virtual void DrawLine(int x0, int y0, int x1, int y1, LineDrawer *drawer);

  virtual void CopyFrontBufferToBackBuffer();

  void FrontBufferRectangle(RECT *area, BYTE color);
  void ClearBorderArea();

  void WriteToFrontBuffer(Font *font, int x, int y,
                          const char *str, BYTE color, BYTE back_color);

  virtual void AttachScalingClipper();
};
