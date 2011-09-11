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

struct IDirectDrawClipper;

class GfxBasic : public Gfx {
protected:
  /** Areas which are blank on the screen as a result of the virtual
   * buffers not fitting evenly into the screen resolution. */
  Array<RECT> blank_areas;

  /** Areas which should be filled with the border color in order to
   * separate the virtual buffers on the screen. */
  Array<RECT> border_areas;

  /** The clip areas that correspond to each virtual buffer. */
  Array<RECT> clip_areas;

  /** The DirectDraw clippers that correspond to each virtual buffer. */
  Array<AutoComPtr<IDirectDrawClipper> > clippers;

  /** True if there is only one virtual buffer and it takes up the entire
   * screen. */ 
  bool single_buffer_standard;

  BYTE border_color;
  BYTE blank_color;

  static GfxBasic *gfx_basic;

  GfxBasic(HWND hWnd, int mode_width, int mode_height, int refresh_rate,
           int virtual_buffer_width, int virtual_buffer_height);

public:
  virtual ~GfxBasic() {gfx_basic = 0;}

  static inline GfxBasic *Get() {return gfx_basic;}
  static void Initialize(HWND hWnd, int mode_width, int mode_height,
                         int refresh_rate, int virtual_buffer_width,
                         int virtual_buffer_height) {
    gfx_basic = new GfxBasic(hWnd, mode_width, mode_height, refresh_rate,
                             virtual_buffer_width, virtual_buffer_height);
    gfx.reset(gfx_basic);
  }

  class SurfaceBasic : public Surface {
  public:
    SurfaceBasic(Gfx *owner, int width, int height)
      : Surface(owner, width, height,
                dynamic_cast<GfxBasic *>(owner)
                ->CreateOffscreenSurface(width, height)) {}

    virtual void Draw(int x, int y, bool transparency);
    virtual void DrawScale(const RECT *dest_rect, const RECT *source_rect,
                           bool transparency);
    virtual bool Refill();
  };

  virtual std::auto_ptr<Font> LoadFont(HGLOBAL resource_handle,
                                       int font_width, int font_height, 
                                       int first_font_char, int last_font_char);

  virtual void Flip();

  virtual void Lock();

  virtual void SetPalette(PALETTEENTRY *pe, int entry_count,
                          bool visual_effect);

  virtual void CopyFrontBufferToBackBuffer();

  virtual std::auto_ptr<Surface>
  CreateSurface(int w, int h, std::auto_ptr<SurfaceFiller> filler);

  virtual void AttachScalingClipper();

  virtual void SetTargetVirtualBuffer(int index);

  /** Returns a DC handle representing the back buffer. This method returns null
   * if the DC could not be gotten.
   */
  HDC GetDC();

  void ReleaseDC(HDC dc);

};
