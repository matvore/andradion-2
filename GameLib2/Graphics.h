// Graphics.h: interface for the CGraphics class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GRAPHICS_H__15C72424_5E64_11D4_B6FE_0050040B0541__INCLUDED_)
#define AFX_GRAPHICS_H__15C72424_5E64_11D4_B6FE_0050040B0541__INCLUDED_

#include "StdAfx.h"

using std::vector;

namespace NGameLib2
{
  const int DEFAULT_REFRESH = 0; // set refresh rate to zero for adaptor default
		
  const int DOUBLE_BUFFERING = 2;
  const int TRIPLE_BUFFERING = 3;
  const int MAX_BUFFERS = 3;
  const int NO_OFFSCREEN_BUFFER = 1;

  // color depths
  const int TRUE_COLOR_32B = 32;
  const int TRUE_COLOR_24B = 24;
  const int HIGH_COLOR_16B = 16;
  const int PALETTIZED_COLOR_8B = 8;

  // screen transitions
  const int TRANSITION_HORIZONTAL_SWIPE = 0;
  const int TRANSITION_BLINDS = 1;
  const int TRANSITION_JEOPARDY_BLOCKS = 2;
  const int TRANSITION_MELTING = 3;
  const int NUM_TRANSITIONS = 4;

  class CBob;
  class CColorNP;
  class CCompactMap;

  class CGraphics : public CCertifiable
    {
    public:
      // constants to define, respectively, no mode x or 13h support, mode x support, or mode 13h support
      enum {MXS_NONE,MXS_X,MXS_13};

      // enumerating friend
      friend HRESULT WINAPI EnumSurfacesCallback(LPDIRECTDRAWSURFACE,DDSURFACEDESC *,void *);

      // another friend
      friend class CSurfaceLock;

      // construction
      //  allows setting of many of the parameters
      //  that are needed. 
      CGraphics(HWND,
		int allow_mode_x_,
		int mode_width_,int mode_height_,
		int mode_bpp_,
		int num_buffers_,
		const GUID& directdraw_driver = GUID_NULL,
		int mode_refresh_=0,
		DWORD advanced_coop_flags_or_ = 0,
		DWORD advanced_coop_flags_and_ = 0-1);

      // a constructor which allows for specification of
      //  all those certification parameters later on
      //  as separates.  It will create a DirectDraw
      //  object with the default display driver
      CGraphics();

      // destruction; automatically 'uncertifies' if necessary
      ~CGraphics();

      // change to different video mode based on
      //  parameters that have been passed
      virtual int Certify();

      // changes to normal graphics mode
      virtual void Uncertify(); 

      // uncertifies and recertifies with new parameters
      //  although it performs the same routine as if
      //  you uncertified and certified manually, this
      //  way makes your code a little cleaner
      int Recertify(
		    int allow_mode_x_,
		    int mode_width_,int mode_height_,
		    int mode_bpp_,
		    int num_buffers_,
		    int mode_refresh_ = 0,
		    DWORD advanced_coop_flags_or_ = 0,
		    DWORD advanced_coop_flags_and_ = 0-1
		    );

      // takes screenshot and writes it to a file
      //  whose handle is given as a parameter
      // returns zero on success
      //  takes the screen shot of the current_area
      DWORD Screenshot(HANDLE file);

      inline int                  GetTargetBufferIndex     (                ) const;
      inline IDirectDrawSurface2 *GetTargetBufferInterface (                ) const;
      inline void                 SetTargetBuffer          (int buffer_index)      ;
      inline RECT&                TargetScreenArea         (                )      ;
      inline const RECT&          cTargetScreenArea        (                ) const;

      // fill an area of the current target buffer
      HRESULT Rectangle(const CColorNP& c,bool wait_then_blit = true,DWORD behav = 0);
      HRESULT Rectangle(BYTE c,bool wait_then_blit = true,DWORD behav = 0);

      // put the bob using FastBlt (no DirectDrawClipper allowed on target buffer, no scaling)
      //  uses the simple, non-direct-draw clipper for increased efficiency (not a guarantee)
      HRESULT PutFastClip(CBob& bob,bool use_transparency = true,bool wait_then_blit = true,DWORD trans_flags = 0);

      // put the bob using FastBlt (no DirectDrawClipper allowed on target buffer, no scaling)
      //  uses no clipping
      HRESULT PutFast(CBob& bob,bool use_transparency = true,bool wait_then_blit = true,DWORD trans_flags = 0);
		
      // put the bob scaled to the current screen
      //  supports scaling, sends the surface right on over
      //  to the IDirectDrawSurface7 interface of the current
      //  target buffer.  So if the target surface does not have
      //  a  DirectDraw clipper, then you need to be sure that the 
      //  bob (blitter object) will not overlap screen boundaries
      HRESULT Put(CBob& bob,DWORD blt_behaviour_flags,DDBLTFX *blt_fx = NULL,bool wait_then_blit = true);

      // flips the front and back buffers
      //  Gives you just about all the control you could want . . .
      HRESULT Flip(DWORD flags);

      // Performs a screen transition from the front buffer
      //  to the back buffer.  In the end, both surfaces
      //  will have identical data.  This function uses
      //  the BltFast() method, and for this reason it
      //  will fail if a clipper is attached to the front
      //  or back buffer.  Search the constants for 
      //  acceptable values for the transition parameter
      //  If an invalid parameter is passed, an assertion
      //  will fail, and you will know about it!
      void Transition(int transition);

      // Returns a pointer to the IDirectDrawSurface7 for
      //  increased flexibility.  You may want to double-
      //  check to make sure the function you want to use
      //  is not supported by GameLib2
      LPDIRECTDRAWSURFACE2 Buffer(int i);

      // Creates and returns a clipper with the specified clip list
      //  it's the callers responsibility to release it.  This function
      //  should give you all the control you could need, but if you
      //  want to create a clipper that is not associated with a single
      //  display device, use the DirectDraw function: DirectDrawCreateClipper
      LPDIRECTDRAWCLIPPER CreateClipper(const vector<RECT>& list,bool set_hwnd = false) const;

      // Creates and returns a palette; it's your responsibility to
      //  release it.  This function should give you all the control
      //  you need.  If init parameter is NULL, then the palette
      //  will be totally black.
      LPDIRECTDRAWPALETTE CreatePalette(
					bool for_primary,
					PALETTEENTRY *init = NULL,
					bool can_change_256 = true,
					bool vb_sync = false,
					DWORD advanced_flags_or = 0,
					DWORD advanced_flags_and = 0-1
					) const;

      // change or read the rectangle where all
      //  non-directdraw clipping happens.  This
      //  can be faster; the CGraphics class will
      //  do the clipping for you, which should
      //  usually reduce some overhead of using
      //  DirectDrawClipper.  If you want to use
      //  more than one clipper rect (you freak),
      //  you'll have to use a DirectDrawClipper
      RECT& SimpleClipperRect();
      const RECT& cSimpleClipperRect() const;

      // clips a set of line coordinates, and returns true if at least part of it is visible
      //  uses the simple clipping region
      bool ClipLine(int& x0,int& y0,int& x1,int& y1) const;

    protected:

      HRESULT RectangleInternal(DWORD c,bool wait_then_blit,DWORD behav);

      LPDIRECTDRAWSURFACE2 buffers[MAX_BUFFERS]; // the buffers (used shorter name for easier typing)
      LPDIRECTDRAWSURFACE2 current_buffer;
      RECT                 current_area;
      RECT                 simple_clipper_rect;

    private: CertParamB(SIZE ,mode_dim               ,ModeDim             );
    private: CertParamA(HWND ,hWnd                   ,Window              );
    private: CertParamA(int  ,mode_bpp               ,ModeBpp             );
    private: CertParamA(int  ,num_buffers            ,NumBuffers          );
    private: CertParamA(int  ,mode_refresh           ,RefreshRate         );
    private: CertParamA(DWORD,advanced_coop_flags_or ,AdvancedCoopFlagsOr );
    private: CertParamA(DWORD,advanced_coop_flags_and,AdvancedCoopFlagsAnd);
    private: CertParamA(int  ,allow_mode_x           ,ModeXSupport        );

    private:
      // things that used to be part of TMonitor:
      static bool          _15b_color      ;
      static int           bytes_per_pixel ;
      static int           mode_width      ;
      static int           mode_height     ;
      static IDirectDraw2 *dd              ;
      static IDirectDraw4 *dd4             ; // this may be NULL for older versions of DirectX

    public:
      // accessors for things that used to be part of TMonitor:
      inline static IDirectDraw2 *DirectDraw    () { return dd              ;}
      inline static int           BytesPerPixel () { return bytes_per_pixel ;}
      inline static bool          _15bColor     () { return _15b_color      ;}
      inline static int           ModeWidth     () { return mode_width      ;}
      inline static int           ModeHeight    () { return mode_height     ;}
    };

  // here are a whole bunch of inline functions for the CGraphics class
  inline IDirectDrawSurface2 *CGraphics::GetTargetBufferInterface() const
    {
      assert(Certified()); 
      return current_buffer;
    }

  inline void CGraphics::SetTargetBuffer(int buffer_index)
    {
      assert(Certified());
      this->current_buffer = this->buffers[buffer_index];
    }

  inline RECT& CGraphics::TargetScreenArea()
    {
      assert(Certified());
      return this->current_area;
    }

  inline const RECT& CGraphics::cTargetScreenArea() const
    {
      assert(Certified());
      return this->current_area;
    }
} // end of namespace

#endif // !defined(AFX_GRAPHICS_H__15C72424_5E64_11D4_B6FE_0050040B0541__INCLUDED_)
