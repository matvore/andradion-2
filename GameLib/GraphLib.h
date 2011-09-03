#ifndef __GRAPHLIB_H__
#define __GRAPHLIB_H__

// includes:
#include <cassert>
#include <vector>
#include <ddraw.h>
#include "GameLib.h"

namespace GraphLib {
	using namespace std;

	typedef void (*SurfaceLossHandler)(LPDIRECTDRAWSURFACE7);

	HRESULT LastConstructorError();
	void SurfaceLost(LPDIRECTDRAWSURFACE7 x);
	SurfaceLossHandler SetSurfaceLossHandler(SurfaceLossHandler x);

	const char SUR_MSG[] =
		"Surface was lost due possibly to change of modes.";

	const char SUR_CAP[] =
		"Surface Lost";

	// pass true to turn it on, false to turn it off
	void EnableSurfaceRestorationGiveUp(bool enabled);
	// never pass false without having previously called it passing true

	const char CGRAPHCERTERR_OUTOFMEMORY[] = "Could not change video modes due to memory shortage.";

	const int DEFAULT_REFRESH = 0; // set refresh rate to zero for adaptor default
		
	const int DOUBLE_BUFFERING = 2;
	const int NO_OFFSCREEN_BUFFER = 1;

	// color depths
	const int TRUE_COLOR_32B = 32;
	const int TRUE_COLOR_24B = 24;
	const int HIGH_COLOR_16B = 16;
	const int PALETTIZED_COLOR_256C = 8;
	const int PALETTIZED_COLOR_16C = 4;

	// mode x/13
	const int MODE13h = -200;
	const int MODEX_320x200 = 200;
	const int MODEX_320x240 = 240;

	const int MODEX_WIDTH = 320;
	const int MODE13h_HEIGHT = 200;
	const int MODEX_BPP = 8;

	const int MODEX_HEIGHTS[] = {200,240};

	// use these constants if using the primary constructor for CGraphics
	//  to enable Mode13h/ModeX, and make sure that mode_height/mode_width match
	const MODEBPP_13h = -1;
	const MODEBPP_XBUTNOT13h = -2;

	// screen transitions
	const int TRANSITION_HORIZONTAL_SWIPE = 0;
	const int TRANSITION_BLINDS = 1;
	const int TRANSITION_JEOPARDY_BLOCKS = 2;
	const int TRANSITION_MELTING = 3;

	const int MULTIMON_ONLYDISPLAYCREATED = 0;
	const int MULTIMON_FIRSTDISPLAYCREATED = 1;
	const int MULTIMON_NOTFIRSTDISPLAYCREATED = 2;

	// list of the classes in alphabetical order
	class CCertifiable;
	class CColor;
	class CColorMap;
	class CGraphics;
	class CMonitorStatic;
	class CScreenArea;
	
	// picking the current monitor:
	typedef struct {
		int arg_bits;
		int ar_bits;
		int a_bits;
		int mode_width;
		int mode_height;
		LPDIRECTDRAW7 dd;
		int multimon_type;
		// see constants above for valid values of multimon_type
	} TMonitor;

	typedef vector<TMonitor> TMonitorList;

	class CCertifiable {
	public:
		CCertifiable() : is_certified(false) {}

		virtual int Certify() = 0;
		virtual void Uncertify() {is_certified = false;}

		bool IsCertified() const {return is_certified;}
		
	protected:
		bool is_certified;
	};

	// used for classes which can only use one monitor
	class CMonitorStatic { 
	public:
		CMonitorStatic();

		bool OnMyMonitor() const;

		int GetModeWidth() const;
		int GetModeHeight() const;

		void SwitchMonitor(int index);

	protected:
		TMonitor& CurrentMonitor();
		const TMonitor& cCurrentMonitor() const;

		// releases the directdraw associated with the calling class
		//  not the current monitor
		void ReleaseMyDirectDraw(); 

	private:
		const int my_monitor;
		static vector<TMonitor> displays;
		static int current_display;
	};

	class CColor : public CCertifiable, public CMonitorStatic {
	public:
		friend class CGraphics;

		CColor(
			unsigned char r_, // red (0 to 255)
			unsigned char g_, // green
			unsigned char b_, // blue
			unsigned char a_ = 0  // alpha (also 0 to 255)
		);

		CColor();

		// certification
		virtual int Certify();

		// get the color 32-bit/24-bit (must be certified)
		unsigned long Color32() const;

		// get the color 16-bit/15-bit (must be certified)
		unsigned short Color16() const;

		// writers
		void SetColor(unsigned char r_,unsigned char g_,unsigned char b_,unsigned char a_=0);
		void SetRed(unsigned char r_);
		void SetGreen(unsigned char g_);
		void SetBlue(unsigned char b_);
		void SetAlpha(unsigned char a_);

		// readers
		void GetColor(unsigned char& r_,unsigned char& g_,unsigned char& b_,unsigned char& a_) const;
		void GetColor(unsigned char& r_,unsigned char& g_,unsigned char& b_) const;
		unsigned char GetRed() const;
		unsigned char GetGreen() const;
		unsigned char GetBlue() const;
		unsigned char GetAlpha() const;

		CColor& operator =(const CColor& rhs);

	private:
		unsigned char r,g,b,a; // red, green, blue, and alpha, respectively
		
		unsigned long color; // formatted color data
	};

	class CColorMap : public CMonitorStatic {
	public:
		friend class CGraphics;

		CColorMap(CColorMap& parent_);
		CColorMap(int width,int height);
		CColorMap();
		CColorMap(HANDLE bitmap);
		
		~CColorMap();
		
		void Size(int width,int height); // destructive to any current data

		void GetSize(int& width,int& height);
		int GetWidth();
		int GetHeight();

		void Extract(HANDLE bitmap,int x=0,int y=0); // load from a bitmap handle

		// to use this you cannot be a child; when a CColorMap is created, solid black is the default
		void SetTransparentColor(const CColor& trans); 

		CColorMap& operator =(CColorMap& rhs); // creates a clone

		LPDIRECTDRAWSURFACE7 Data();

	private:
		LPDIRECTDRAWSURFACE7 data; // = NULL if no memory allocated
	};

	class CScreenArea : public CCertifiable, public CMonitorStatic {
	public:
		friend class CGraphics;
		
		// constructors
		CScreenArea(int x_, int y_, int w_, int h_);
		CScreenArea();
	
		// certification
		virtual int Certify();

		RECT Area() const; // must be certified to translate

		// readers
		int GetLeft() const;
		int GetWidth() const;
		int GetTop() const;
		int GetHeight() const;
		void GetArea(int &x_,int &y_,int &w_,int &h_) const;

		// writers
		void SetLeft(int x_);
		void SetWidth(int w_);
		void SetTop(int y_);
		void SetHeight(int h_);
		void SetArea(int x_,int y_,int w_,int h_);

		CScreenArea& operator =(const CScreenArea& rhs);

	private:
		int x,y,w,h;
	};

	class CGraphics : public CCertifiable, public CMonitorStatic { // "certifying" involves initializing graphics
	public:
		// enumeratiing friend
		friend HRESULT WINAPI EnumSurfacesCallback2( 
			LPDIRECTDRAWSURFACE7 lpDDSurface,  
			LPDDSURFACEDESC2 lpDDSurfaceDesc,  
			LPVOID cgraphics);

		// construction
		CGraphics(
			HWND hWnd_,
			int mode_width,int mode_height,
			int mode_bpp_,
			int num_buffers_,
			GUID *directdraw_driver_,
			int mode_refresh_,
			int multimon_type = MULTIMON_ONLYDISPLAYCREATED
			// see constants above for valid values of multimon_type
		);

		// mode X or 13h construction
		CGraphics(
			HWND hWnd_,
			int mode,
			int num_buffers_,
			GUID *directdraw_driver_,
			int mode_refresh_,
			int multimon_type = MULTIMON_ONLYDISPLAYCREATED
		);

		// destruction
		~CGraphics();

		// certification
		virtual int Certify();
		virtual void Uncertify(); // closes graphics

		// writing
		void SetWindow(HWND hWnd_);
		void SetMode(int mode); // 13h or mode X
		void SetMode(int mode_width,int mode_height,int mode_bpp_);
		void SetNumBuffers(int num_buffers_);
		void SetRefreshRate(int mode_refresh_);
		
		// reading
		bool ModeIs13h() const;
		bool ModeIsX() const;
		int GetModeBpp() const;
		HWND GetWindow() const;
		int GetNumBuffers() const;
		const GUID *GetDirectDrawDriver() const;
		int GetRefreshRate() const;

		// rendering and other graphics operations

		// target buffer changing
		void SetTargetBuffer(int buffer_index);

		// target buffer finding
		int GetTargetBuffer() const;

		// accessing the target screen area
		CScreenArea& TargetScreenArea();

		// reading the target screen area
		const CScreenArea& cTargetScreenArea() const;

		// fill a small area of the current buffer
		void Rectangle(const CColor& c);

		// put the bob scaled to the current screen
		void Put(CColorMap& bob,bool use_transparency = true);

		// flip (uses buffer 0 always)
		void Flip();
		void Transition(int transition);

		LPDIRECTDRAWSURFACE7 Buffer(int i)
		{
			assert(i >= 0 && i < this->num_buffers && true == this->is_certified && this->OnMyMonitor());
			return b[i];
		}
		
		LPDIRECTDRAWCLIPPER Clipper()
		{
			assert(true == this->is_certified && this->OnMyMonitor());
			return c;
		}

		LPDIRECTDRAW7 DirectDraw()
		{
			assert(true == this->is_certified && this->OnMyMonitor());
			return this->CurrentMonitor().dd;
		}

	protected:
		LPDIRECTDRAWSURFACE7 *b; // the buffers (short name)
		LPDIRECTDRAWCLIPPER c; // the clipper
		
		HWND hWnd;
		
		int mode_bpp; // mode width and height stored elsewhere

		// for signification of mode X/13h:
		// set mode_bpp = -1 for 13h
		// set mode_bpp = -2 for X
		// mode_width and mode_height of the current display must be
		//  set accordingly
		// if mode_bpp = 0, then the game is run in a window
		
		int num_buffers;

		GUID *directdraw_driver;

		int mode_refresh;

		CScreenArea current_area;
		int current_buffer;
	};
}

#endif
