/*
Copyright 2011 Matt DeVore

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

/** Provides an interface to the graphics hardware of the system.
 * This is an abstract superclass. Generally, only one constructed
 * instance of the class may exist at a time, because as soon as an
 * object of this type is created, the graphics mode is changed.
 */
class Gfx {
public:
  class Surface;

  /** Represents an object that is related to a Gfx object in
   * some way.
   */
  class OwnedByGraphics {
  protected:
    Gfx *owner;

  public:
    OwnedByGraphics(Gfx *owner) : owner(owner) {}
    virtual ~OwnedByGraphics() {}

    Gfx *GetOwner() {return owner;}

    template <class G>
    G *GetOwner() {return dynamic_cast<G *>(owner);}
  };

  class Exception : public std::exception {
  public:
    const int line;
    const HRESULT error_code;
    const std::string message;

    Exception(int line, HRESULT error_code) throw();
    Exception(int line, const std::string& message) throw();

    virtual ~Exception() throw() {}

    virtual const char *what() const throw() {return message.c_str();}
  };

  /** Represents an object that knows how to supply a Surface with
   * some kind of bitmap data. This object is supplied with a pointer
   * to the surface's data - the region of this pointer must be
   * written to in order to form the image of the surface.
   * A valid SurfaceFiller implementation does not have to specify
   * every pixel in the surface; it may leave one or more pixels
   * unspecified, in which case the color of that pixel once the
   * surface is displayed in undefined.
   */
  class SurfaceFiller {
  public:
    virtual ~SurfaceFiller() {}

    /** Fills the given surface with some kind of bitmap data.
     * @param surf_ptr a pointer to the upper-left pixel of the
     * @param pitch the 'pitch' of the region specified by surf_ptr.
     *  The pitch indicates the number of bytes from the first pixel
     *  of one row to the first pixel of the next row.
     * @param surf the surface object that is being filled.
     *  This object can be used to obtain the width and height of the
     *  surface. Note that the owner of the Surface may differ between
     *  calls to this method.
     */
    virtual void Fill(BYTE *surf_ptr, int pitch,
                      Surface *surf) = 0;
  };

  /** An implementation of SurfaceFiller which leaves all pixels in
   * the surface as undefined; this should be useful when the 'correct'
   * SurfaceFiller for a surface is not yet known.
   */
  class NilSurfaceFiller : public SurfaceFiller {
  public:
    virtual void Fill(BYTE *, int pitch, Surface *) {}
  };

  /** Represents a two-dimensional image of a rectangular shape,
   * though the image does not have to appear as a rectangle on screen
   * if transparency is used. The image data is contained in a
   * DirectDraw surface, whose actual (physical) dimensions may not be
   * the same as the virtual dimensions, which is the size that which this
   * surface is treated as having.
   */
  class Surface : public OwnedByGraphics {
    friend class Gfx;

  protected:
    /** Keeps COM initialized while this object is alive. */
    Com com;

    /** Contains the image data of this Surface object. */
    AutoComPtr<IDirectDrawSurface> surface;

    /** Never a null pointer, this object is given the chance to
     * 'fill' the surface whenever it is loaded, or some other
     * important event happens.
     */
    std::auto_ptr<SurfaceFiller> filler;

    /** The virtual width of this surface. */
    int width;

    /** The virtual height of this surface. */
    int height;

  public:
    /** Initializes the members of this object to the given values.
     * Each parameter is assigned to the member of the same name.
     * @param width the virtual width of the new surface.
     * @param height the virtual height of the new surface.
     */
    Surface(Gfx *owner, int width, int height,
            AutoComPtr<IDirectDrawSurface> surface)
      : OwnedByGraphics(owner), width(width), height(height),
        filler(new NilSurfaceFiller()), surface(surface) {
      owner->surfaces.insert(this);
    }

    virtual ~Surface();

    /** Draws this surface to the back buffer of the Gfx object
     * that owns this object. This surface is drawn to the coordinates
     * given, clipped to the sides of the screen, and with or without
     * transparency as specified. The default implementation uses the
     * BltFast method of the DirectDrawSurface object and a simple
     * clipping algorithm.
     */
    virtual void Draw(int x, int y, bool transparency) = 0;

    /** Draws a section of the surface to the back buffer of the
     * Gfx object that owns this object. The area occupied in the
     * back buffer by this object's image is specified in
     * dest_rect. The section of this object's image that is drawn is
     * given by source_rect. The default implementation uses a single
     * call to the Blt method of the DirectDrawSurface object.
     */
    virtual void DrawScale(const RECT *dest_rect, const RECT *source_rect,
                           bool transparency) = 0;

    /** Supplies the width in pixels of this surface.
     * @return the (virtual) width of this surface.
     */
    int GetWidth() {return width;}

    /** Supplies the height in pixels of this surface.
     * @return the (virtual) height of this surface.
     */
    int GetHeight() {return height;}

    /** Allows changing of the surface filler of this surface. The
     * surface is not refilled by the new filler until the Refill()
     * method is called.
     * @param an auto pointer to the new surface filler. The auto
     *  pointer cannot point to null.
     * @return an auto pointer to the previous surface filler.
     */
    std::auto_ptr<SurfaceFiller>
    ChangeFiller(std::auto_ptr<SurfaceFiller> new_filler) {
      SurfaceFiller *old = filler.release();

      filler = new_filler;

      Refill();

      return std::auto_ptr<SurfaceFiller>(old);
    }

    /** Uses the current surface filler to fill the surface with image
     * data. Before it uses the image filler, this method will first
     * restore the surface if it is lost.
     * The default implementation is incomplete; it only restores the surface
     * without invoking the surface filler.
     * @return true if the surface could be restored or if it was already
     *  restored before the surface was called.
     */
    virtual bool Refill();
  };

  class Font : public OwnedByGraphics {
  protected:
    typedef Array<BYTE> Character;
    typedef Array<Character> CharacterSet;

    CharacterSet font_data;

    int font_width;
    int font_height;

    int first_font_char;
    int last_font_char;

  public:
    int GetCharWidth() {return font_width;}
    int GetCharHeight() {return font_height;}

    Font(Gfx *owner, void *resource_data, int font_width,
         int font_height, int first_font_char, int last_font_char);

    void WriteString(int x, int y, const char *str, BYTE color);
    virtual void WriteChar(int x, int y, int ch, BYTE color);
  };

  class BitmapSurfaceFiller : public SurfaceFiller {
  protected:
    HINSTANCE hInst;
    LPCTSTR res_name;
    int width;
    int height;

    HBITMAP LoadImage(int wanted_width = 0, int wanted_height = 0);

  public:
    BitmapSurfaceFiller(HINSTANCE hInst, LPCTSTR res_name);

    virtual void Fill(BYTE *surf_ptr, int surf_pitch, Surface *surface);

    int GetWidth() {if (width<0) {DeleteObject(LoadImage());} return width;}
    int GetHeight() {if (height<0) {DeleteObject(LoadImage());} return height;}

    static std::auto_ptr<Surface> CreateSurfaceFromBitmap
    (Gfx *owner, HINSTANCE hInstance, LPCTSTR resource_name);
  };

  class LineDrawer {
  public:
    virtual ~LineDrawer() {}

    virtual void BeginLine() = 0;
    virtual int GetNextPixel() = 0;
  };

protected:
  class SurfaceLock {
  protected:
    int pitch;
    BYTE *surface_ptr;
    IDirectDrawSurface *surface;

  public:
    SurfaceLock(IDirectDrawSurface *surface);
    ~SurfaceLock();

    inline BYTE *GetPtr() {return surface_ptr;}
    inline BYTE *GetPtr(int x, int y) {return surface_ptr + x + y * pitch;}

    inline int GetPitch() {return pitch;}
  };

  std::set<Surface *> surfaces;

  /** If the DirectDraw device supports asynchronous blits, this is set to
   * DDBLT_ASYNC. Otherwise, it is set to DDBLT_WAIT.
   */
  DWORD async_blit_flags;

  AutoComPtr<IDirectDraw2> direct_draw;

  AutoComPtr<IDirectDrawSurface> front_buffer;
  /** The width of the front buffer. There is no "virtual" width for the front
      buffer. The dimensions of the front buffer are equal to the video mode
      resolution. */
  int mode_width;

  /** The height of the front buffer. There is no "virtual" height for
      the front buffer. The dimensions of the front buffer are equal to the
      video mode resolution. */
  int mode_height;

  /** The back buffer. The back buffer may not be in a flip chain with the front
   * buffer; it may simply be another buffer. It may or may not be the same size
   * as the front buffer.
   */
  AutoComPtr<IDirectDrawSurface> back_buffer;

  int target_virtual_buffer;
  int virtual_buffer_count;

  /** The width of the virtual buffers. */
  int virtual_buffer_width;

  /** The height of the virtual buffers. */
  int virtual_buffer_height;

  /** When locked, a pointer to the upper-left pixel of the current virtual
   * buffer. When not locked, this pointer is null.
   */
  BYTE *lock_ptr;

  /** When locked, the pitch of the current virtual buffer surface data.
   * The functionality of the Gfx library will use this pitch to
   * draw lines and write characters to the screen. This value can be
   * doubled to draw only to every other line.
   */
  int lock_pitch;

  /** The number of Lock calls that have been made that have not been
   * canceled out with a call to Unlock.
   */
  int lock_count;

  /** Holds the Lock pointer to the Lock we have, if any, on the back
   * buffer. The pitch and surface pointer of this lock is physical, because it
   * represents the exact values gotten from the DirectDraw Lock operation. The
   * virtual values are stored in the lock_pitch and lock_ptr member data.
   */
  std::auto_ptr<SurfaceLock> lock;

  PALETTEENTRY palette[256];

  int clip_count;

  /** Creates an offscreen Direct Draw surface with the given physical
   * dimensions. Throws an Exception if the surface could not be
   * created.
   */
  AutoComPtr<IDirectDrawSurface> CreateOffscreenSurface(int w, int y);

  /** Creates a DirectDrawClipper that clips to the specified area. Null is
   * returned on failure.
   */
  AutoComPtr<IDirectDrawClipper> CreateClipper(const RECT *area);

  /** Clips the given coordinates as if they were being used to draw a
   * surface.
   * The region which to clip against is passed in the clip_area parameter.
   * @param dest_abs_x Where the surface is about to be blitted to the
   *  back buffer (x coordinate). Upon function return, this value
   *  will equal the x-coordinate at which the surface should be
   *  blitted.
   * @param dest_abs_y Where the surface is about to be blitted to the
   *  back buffer (y coordinate). Upon function return, this value
   *  will equal the y-coordinate at which the surface should be
   *  blitted.
   * @param source the top and left values of this RECT struct must be
   *  initialized to zero, and the other values to the physical size
   *  of the surface being drawn from; before the method is
   *  called. Once the method returns, this will specify in physical
   *  coordinates the area of the source surface to be blitted.
   * @param clip_area the area against which to clip.
   * @return true if at least one pixel of the surface should be
   * visible on the screen.
   */
  static bool Clip(int *dest_abs_x, int *dest_abs_y, RECT *source,
                   const RECT *clip_area);

  void RefillSurfaces();

  static void DoOrFail(int line, HRESULT hr) throw(Exception);

  static AutoComPtr<IDirectDraw2> CreateDirectDraw();

  static std::auto_ptr<Gfx> gfx;

  Gfx(HWND hWnd, int mode_width, int mode_height,
      int refresh_rate, bool create_back_buffer,
      int virtual_buffer_width, int virtual_buffer_height,
      int virtual_buffer_count);

public:
  virtual ~Gfx();

  // singleton-related methods
  static inline Gfx *Get() {return gfx.get();}
  // static void Initialize(...) {...}
  static void Release() {gfx.reset();}

  int GetVirtualBufferWidth() {return virtual_buffer_width;}
  int GetVirtualBufferHeight() {return virtual_buffer_height;}

  int GetModeWidth() {return mode_width;}
  int GetModeHeight() {return mode_height;}

  virtual std::auto_ptr<Font>
  LoadFont(HGLOBAL resource_handle, int font_width,
           int font_height, int first_font_char, int last_font_char) = 0;

  /** Makes the back buffer visible to the user. The default implementation of
   * this method simply attempts to restore and reload all the surfaces if they
   * have been lost.
   */
  virtual void Flip();

  virtual void Lock();
  void Unlock();
  BYTE *GetLockPtr() {assert(lock_ptr); return lock_ptr;}
  BYTE *GetLockPtr(int x, int y) {return lock_ptr + x + y * lock_pitch;}
  int GetLockPitch() {assert(lock_ptr); return lock_pitch;}

  /** Draws a rectangle to the current virtual buffer.
   * The lock must be active.
   */
  virtual void Rectangle(const RECT *area, BYTE color, bool clip);

  virtual void SetPalette(PALETTEENTRY *pe, int entry_count,
                          bool visual_effect);
  BYTE MatchingColor(COLORREF rgb);
  bool InFocus();
  void FlipToGDISurface();

  /** Transfers the contents of the front buffer to a bitmap specified by the
   * given filename. If the operation fails, a corrupt file may be left in the
   * place of the given filename.
   *
   * @return true if the operation succeeded, false otherwise.
   */
  bool Screenshot(const char *filename);

  virtual void CopyFrontBufferToBackBuffer() = 0;

  const PALETTEENTRY *GetPalette() {return palette;}

  /** Not only does this method create the surface and add it to the surfaces
   * roster, it also fills it with the specified surface filler.
   * @param filler the surface filler with which to fill the surface now, and at
   *  all times in the future when the palette changes and when the surface is
   *  restored after being lost.
   */
  virtual std::auto_ptr<Surface> CreateSurface
  (int w, int h, std::auto_ptr<SurfaceFiller> filler) = 0;

  virtual void AttachScalingClipper() = 0;
  void DetachScalingClipper();

  /** Clips the given line to the current virtual buffer.
   * @return true if the line will be visible if drawn, false if not.
   */
  bool ClipLine(int *x0, int *y0, int *x1, int *y1);
  virtual void DrawLine(int x0, int y0, int x1, int y1, LineDrawer *drawer);

  int GetVirtualBufferCount() {return virtual_buffer_count;}
  virtual void SetTargetVirtualBuffer(int index) {
    assert(index >= 0 && index < virtual_buffer_count);
    target_virtual_buffer = index;
  }
  int GetTargetVirtualBuffer() {return target_virtual_buffer;}

  inline static void Rectangle
  (BYTE *surf_ptr, int pitch, int x, int y, int w, int h, BYTE color) {
    surf_ptr += x + y * pitch;

    while (h-- > 0) {
      memset(surf_ptr, color, w);

      surf_ptr += pitch;
    }
  }

  /** Returns true if an 8-bit non-Mode-X display mode of the given dimensions
   * is available  on the DirectDraw device. If there was some error in
   * determining for certain if the mode is available, this method returns
   * false.
   */
  static bool VideoModeAvailable(int mode_width, int mode_height);
};
