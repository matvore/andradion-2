// the GraphLib graphics library
#include "GameLib.h"
#include "GraphLib.h"
#include "Timer.h"
#include <ctime>

using namespace GenericClassLib;

using namespace std;

namespace GraphLib {
	// some static members
	int CMonitorStatic::current_display = 0;
	std::vector<TMonitor> CMonitorStatic::displays;
	
	static SurfaceLossHandler slh = NULL;
	
	static HRESULT last_constructor_error;

	static void SurGiveUp(LPDIRECTDRAWSURFACE7)
	{
		MessageBox(NULL,SUR_MSG,SUR_CAP,MB_ICONSTOP);
		assert(false);

		exit(1);
	}

	void EnableSurfaceRestorationGiveUp(bool enabled)
	{
		static SurfaceLossHandler old_handler = NULL;

		if(true == enabled) // enable lazy error handling
		{
			old_handler = SetSurfaceLossHandler(SurGiveUp);
		}
		else
		{
			SetSurfaceLossHandler(old_handler);
		}
	}

	HRESULT LastConstructorError()
	{
		return last_constructor_error;
	}

	void SurfaceLost(LPDIRECTDRAWSURFACE7 x)
	{
		assert(NULL != slh);
		slh(x);
	}

	SurfaceLossHandler SetSurfaceLossHandler(SurfaceLossHandler x)
	{
		SurfaceLossHandler ret = slh;
		slh = x;
		return ret;
	}

	// useful to find the number of bits stored in a given piece of data
	static int NumberOfBitsOn(DWORD data) {
		int ret = 0;
		
		for(int i = 0; i < 32; i++)
			if((data >> i) & 1) ret++;
			
		return ret;
	}

	static HRESULT WINAPI EnumSurfacesCallback2( 
		LPDIRECTDRAWSURFACE7 lpDDSurface,  
		LPDDSURFACEDESC2 lpDDSurfaceDesc,  
		LPVOID cgraphics
		) {
		CGraphics *g;
		
		g = (CGraphics *)cgraphics;

		g->b[g->current_buffer++] = lpDDSurface;

		return DDENUMRET_OK; // continue the enumeration
	}

	CMonitorStatic::CMonitorStatic() : my_monitor(current_display) {
		if(displays.size() == 0) // if we haven't sized the displays vector yet
			displays.resize(1); // then make it 1 big now
	}

	bool CMonitorStatic::OnMyMonitor() const {
		return (my_monitor == current_display) ?
			true : false;
	}

	int CMonitorStatic::GetModeWidth() const {
		assert(true == this->OnMyMonitor());
		return displays[current_display].mode_width;
	}

	int CMonitorStatic::GetModeHeight() const {
		assert(true == this->OnMyMonitor());
		return displays[current_display].mode_height;
	}

	void CMonitorStatic::SwitchMonitor(int index) {
		current_display = index;
		if(displays.size() <= index)
			displays.resize(index+1);
	}

	TMonitor& CMonitorStatic::CurrentMonitor() {
		assert(true == this->OnMyMonitor());
		return displays[current_display];
	}

	const TMonitor& CMonitorStatic::cCurrentMonitor() const {
		assert(true == this->OnMyMonitor());
		return displays[current_display];
	}

	void CMonitorStatic::ReleaseMyDirectDraw() {
		displays[my_monitor].dd->Release(); // there we go
	}

	CColor::CColor(
		unsigned char r_,unsigned char g_,
		unsigned char b_,unsigned char a_
	) :
	r(r_),g(g_),b(b_),a(a_), CCertifiable(), CMonitorStatic() {}

	CColor::CColor() : CCertifiable(), CMonitorStatic() {}

	int CColor::Certify() {
		switch(cCurrentMonitor().arg_bits) { // this val can say a lot
		case 24: // 32-bit
			color = 
				(a << 24) +
				(r << 16) +
				(g << 8) +
				(b);

			break;
		case 16: // 24-bit
			color =
				(r << 16) +
				(g << 8) +
				(b);

			break;
		case 11: // 16/15-bit
			if(cCurrentMonitor().a_bits)  // 15bpp + 1a
				color =
					((a>>7)<<15) +
					((r>>3)<<10)+
					((g>>3)<<5);					
			else // 16bpp + 1g
				color =
					((r>>3)<<11)+
					((g>>2)<<5);
					
			color += b>>3;
		}

		// colors can always be certified
		is_certified = true;

		return 0;
	}

	unsigned long CColor::Color32() const {
		assert(is_certified && true == this->OnMyMonitor());
		return color;
	}

	unsigned short CColor::Color16() const {
		assert(is_certified && true == this->OnMyMonitor());
		return (unsigned short)color;
	}

	void CColor::SetColor(
		unsigned char r_,
		unsigned char g_,
		unsigned char b_,
		unsigned char a_
		) {
		r = r_;
		g = g_;
		b = b_;
		a = a_;
		is_certified = false;
	}

	void CColor::SetRed(unsigned char r_) {
		r = r_;
		is_certified = false;
	}

	void CColor::SetGreen(unsigned char g_) {
		g = g_;
		is_certified = false;
	}

	void CColor::SetBlue(unsigned char b_) {
		b = b_;
		is_certified = false;
	}

	void CColor::SetAlpha(unsigned char a_) {
		a = a_;
		is_certified = false;
	}

	void CColor::GetColor(
		unsigned char& r_,
		unsigned char& g_,
		unsigned char& b_
		,unsigned char& a_) const {
		r_ = r;
		g_ = g;
		b_ = b;
		a_ = a;
	}

	void CColor::GetColor(
		unsigned char& r_,
		unsigned char& g_,
		unsigned char& b_
		//,unsigned char& a_) const {
		) const {
		r_ = r;
		g_ = g;
		b_ = b;
		//a_ = a;
	}

	unsigned char CColor::GetRed() const {
		return r;
	}

	unsigned char CColor::GetGreen() const {
		return g;
	}

	unsigned char CColor::GetBlue() const {
		return b;
	}

	unsigned char CColor::GetAlpha() const {
		return a;
	}

	CColor& CColor::operator =(const CColor& rhs)
	{
		if(this == &rhs) return *this;
		r = rhs.r;
		g = rhs.g;
		b = rhs.b;
		a = rhs.a;
		is_certified = false;
		return *this;
	}

	CColorMap::CColorMap(CColorMap& parent_):CMonitorStatic()
	{
		this->data = NULL;
		this->operator =(parent_);
	}

	CColorMap::CColorMap(HANDLE bitmap):CMonitorStatic()
	{
		BITMAP bmp;

		data = NULL;
		GetObject(bitmap,sizeof(BITMAP),&bmp); // get the passed bitmap size
		Size(bmp.bmWidth,bmp.bmHeight); // resize according to the size of the bitmap
		
		Extract(bitmap,0,0);
	}

	CColorMap::CColorMap() : data(NULL), 
		CMonitorStatic() {}
		// we're empty!

	CColorMap::CColorMap(int width,int height):CMonitorStatic()
	{
		data = NULL;
		Size(width,height);
	}

	CColorMap::~CColorMap()
	{
		data->Release();
	}

	void CColorMap::Size(int width,int height) {
		DDSURFACEDESC2 d;
		CColor black(0,0,0,0);
		HRESULT res;
		
		assert(NULL == this->data);
		assert(true == this->OnMyMonitor()); // we can have no data!

		memset((void *)&d,0,sizeof(d)); // zero out d

		d.dwSize = sizeof(d); // set the size member

		// set the dimensions of the surface description
		d.dwHeight = height;
		d.dwWidth = width;

		// set surface to offscreen plain in video memory
		d.ddsCaps.dwCaps =
			DDSCAPS_OFFSCREENPLAIN |
			DDSCAPS_VIDEOMEMORY |
			DDSCAPS_LOCALVIDMEM;

		// set the flags:
		d.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;

		// try to create the surface
		if(FAILED(this->CurrentMonitor().dd->CreateSurface(&d,&data,NULL))) {

			// if we fail, try to allocate in non-local video memory
			d.ddsCaps.dwCaps &=
				~DDSCAPS_LOCALVIDMEM; // turn that off
			d.ddsCaps.dwCaps |=
				DDSCAPS_NONLOCALVIDMEM; // turn that on

			if(FAILED(this->CurrentMonitor().dd->CreateSurface(&d,&data,NULL))) {
				// now try to allocate in system memory
				d.ddsCaps.dwCaps &=
					~(DDSCAPS_NONLOCALVIDMEM|DDSCAPS_VIDEOMEMORY); 
				d.ddsCaps.dwCaps |=
					DDSCAPS_SYSTEMMEMORY;
				Memory_Alloc_Function(
					res = this->CurrentMonitor().dd->CreateSurface(&d,&data,NULL),
					width * height * (this->CurrentMonitor().arg_bits + this->CurrentMonitor().ar_bits - this->CurrentMonitor().a_bits),
					FAILED(res)
				);
			}
		}

		black.Certify();
		this->SetTransparentColor(black);
	}

	void CColorMap::GetSize(int& width,int& height) {
		assert(NULL != data); // we need data! we need to be initialized!

		DDSURFACEDESC2 d; // our descriptor

		memset((void *)&d,0,sizeof(d)); // zero out d

		d.dwSize = sizeof(d); // set the data
 
 		data->GetSurfaceDesc(&d); // get the data

		width = d.dwWidth;
		height = d.dwHeight;
	}

	int CColorMap::GetWidth() {
		assert(data); // we need data! we need to be initialized!

		DDSURFACEDESC2 d; // our descriptor

		memset((void *)&d,0,sizeof(d)); // zero out d

		d.dwSize = sizeof(d); // set the data
 
 		data->GetSurfaceDesc(&d); // get the data

		return d.dwWidth;
	}

	int CColorMap::GetHeight() {
		assert(data); // we need data! we need to be initialized!

		DDSURFACEDESC2 d; // our descriptor

		memset((void *)&d,0,sizeof(d)); // zero out d

		d.dwSize = sizeof(d); // set the data
 
 		data->GetSurfaceDesc(&d); // get the data

		return d.dwHeight;
	}

	void CColorMap::Extract(HANDLE bitmap,int x,int y) {
		HDC me;
		int w,h;

		this->GetSize(w,h); // get our size

		// need to have data; cannot have children
		assert(NULL != data); 

		// get the destination dc
		while(FAILED(data->GetDC(&me)))
		{
			SurfaceLost(this->data);
		}

		// make compatible
		HDC bit_dc=CreateCompatibleDC(me);
		SelectObject(bit_dc,bitmap);
		
		// copy
		BitBlt(me,0,0,w,h,bit_dc,x,y,SRCCOPY);

		DeleteDC(bit_dc); // get rid of compatible dc
		
		// release our dc
		while(FAILED(data->ReleaseDC(me)))
		{
			SurfaceLost(this->data);
		}
	}

	CColorMap& CColorMap::operator =(CColorMap& rhs) { // become a child
		// we can't be initialized yet
		assert(NULL == data && true == rhs.OnMyMonitor() && true == this->OnMyMonitor()); 

		if(this == &rhs)
		{
			return *this; // can't assign to self
		}

		while(FAILED(this->CurrentMonitor().dd->DuplicateSurface(rhs.data,&(this->data))))
		{
			SurfaceLost(rhs.data);
		}

		return *this; // return ourself
	}

	void CColorMap::SetTransparentColor(const CColor& trans) {
		DDCOLORKEY k;
		assert(true == this->OnMyMonitor() && true == trans.OnMyMonitor() && data);

		k.dwColorSpaceHighValue = k.dwColorSpaceLowValue =
			trans.Color32();

		while( // try to change the key
			FAILED(data->SetColorKey(DDCKEY_SRCBLT,&k))
		)
		{
			SurfaceLost(this->data);
		}

		// we're done!
	}

	LPDIRECTDRAWSURFACE7 CColorMap::Data() {
		assert(true == this->OnMyMonitor());
		assert(NULL !=data);
		return data;
	}

	CScreenArea::CScreenArea(int x_,int y_,int w_,int h_) : CCertifiable(), CMonitorStatic() {
		x = x_;
		y = y_;
		w = w_;
		h = h_;
	}

	CScreenArea::CScreenArea() : CCertifiable(), CMonitorStatic() {
		x = y = w = h = 0;
	}

	int CScreenArea::Certify() {
		assert(true == this->OnMyMonitor());
		assert(w > 0);
		assert(h > 0);

		is_certified= true;

		return 0;
	}

	int CScreenArea::GetHeight() const {
		return h;
	}

	int CScreenArea::GetLeft() const {
		return x;
	}

	int CScreenArea::GetTop() const {
		return y;
	}

	int CScreenArea::GetWidth() const {
		return w;
	}

	CScreenArea& CScreenArea::operator =(const CScreenArea& rhs) {
		if(this == &rhs) return *this;
		x = rhs.x;
		y = rhs.y;
		w = rhs.w;
		h = rhs.h;
		return *this;
	}

	void CScreenArea::GetArea(int& x_,int& y_,int& w_,int& h_) const {
		assert(true == this->OnMyMonitor() && true == this->is_certified);
		x_ = x;
		y_ = y;
		w_ = w;
		h_ = h;
	}

	void CScreenArea::SetArea(int x_,int y_,int w_,int h_) {
		x = x_;
		y = y_;
		w = w_;
		h = h_;
		is_certified = false;
	}

	RECT CScreenArea::Area() const {
		RECT r;
		
		assert(true == this->OnMyMonitor() && true == this->is_certified);

		r.top = y;
		r.left = x; 
		r.right = x + w;
		r.bottom = y + h;

		return r;
	}

	void CScreenArea::SetHeight(int h_) {
		h = h_;
		is_certified = false;
	}

	void CScreenArea::SetLeft(int x_) {
		x = x_;
		is_certified = false;
	}

	void CScreenArea::SetTop(int y_) {
		y = y_;
		is_certified = false;
	}

	void CScreenArea::SetWidth(int w_) {
		w = w_;
		is_certified = false;
	}

	CGraphics::CGraphics(
		HWND hWnd_,int mode_width,int mode_height,
		int mode_bpp_,int num_buffers_,GUID *directdraw_driver_,
		int mode_refresh_,int multimon_type) :
	CCertifiable(), CMonitorStatic(),
	hWnd(hWnd_),current_buffer(0),directdraw_driver(directdraw_driver_),
	mode_bpp(mode_bpp_),mode_refresh(mode_refresh_),num_buffers(num_buffers_)
	{
		CurrentMonitor().mode_width = mode_width;
		CurrentMonitor().mode_height = mode_height;
		CurrentMonitor().multimon_type = multimon_type;

		// create the directdraw instance
		CoInitialize(NULL);
		CoCreateInstance
		(
			CLSID_DirectDraw,
			NULL,
			CLSCTX_ALL,
			IID_IDirectDraw7,
			(void **)&this->CurrentMonitor().dd
		);
		
		last_constructor_error = 
			this->CurrentMonitor().dd->Initialize(this->directdraw_driver);

		if(FAILED(last_constructor_error))
		{
			return;
		}

		// try to create the clipper
		Memory_Alloc_Function(
			last_constructor_error = this->CurrentMonitor().dd->CreateClipper(0,&c,NULL),
			1,
			FAILED(last_constructor_error)
		);
			
		// now we're at the end
	}

	CGraphics::CGraphics(HWND hWnd_,
		int mode,int num_buffers_,
		GUID *directdraw_driver_,
		int mode_refresh_,
		int multimon_type) :
	hWnd(hWnd_),num_buffers(num_buffers_),directdraw_driver(directdraw_driver_),
	mode_refresh(mode_refresh_)
	{
		HRESULT res;

		CurrentMonitor().mode_width = MODEX_WIDTH; // width is constant
		CurrentMonitor().mode_height = 
			(mode == MODEX_320x240) ? MODEX_HEIGHTS[1] : MODEX_HEIGHTS[0];
		mode_bpp =
			(mode == MODE13h) ? MODEBPP_13h : MODEBPP_XBUTNOT13h;
		CurrentMonitor().multimon_type = multimon_type;
		
		// create the directdraw instance
		// create the directdraw instance
		CoInitialize(NULL);
		CoCreateInstance
		(
			CLSID_DirectDraw,
			NULL,
			CLSCTX_ALL,
			IID_IDirectDraw7,
			(void **)&this->CurrentMonitor().dd
		);
		
		last_constructor_error = 
			this->CurrentMonitor().dd->Initialize(this->directdraw_driver);

		if(FAILED(last_constructor_error))
		{
			return;
		}

		// try to create the clipper
		Memory_Alloc_Function(
			res = CurrentMonitor().dd->CreateClipper(0,&c,NULL),
			1,
			FAILED(res)
		);
	}

	CGraphics::~CGraphics() {
		if(true == this->is_certified)
		{
			this->Uncertify(); // terminate our graphics mode
		}
		
		c->Release(); // get rid of the clipper

		ReleaseMyDirectDraw(); // and then the directdraw interface

		CoUninitialize();
	}

	int CGraphics::Certify() {
		HRESULT res; // result of some ddraw calls
		DDSURFACEDESC2 surf;
		LPRGNDATA cr; // clipper region
		RECT scr; // the whole screen (used for filling out cr)
		DDPIXELFORMAT pf; // pixel format of the new surfaces

		if(true == this->is_certified)
		{
			return 0; // don't certify twice
		}

		// check values of various parameters
		assert(mode_bpp >= MODEBPP_XBUTNOT13h);
		assert(mode_refresh >= 0);
		assert(num_buffers > 0);
		assert(CurrentMonitor().mode_width >= 1);
		assert(CurrentMonitor().mode_height >= 1);

		if(MULTIMON_FIRSTDISPLAYCREATED == CurrentMonitor().multimon_type)
		{
			// if this is the first display created out of numerous displays,
			//  we have to set the focus window
			Memory_Alloc_Function(
				res = CurrentMonitor().dd->SetCooperativeLevel(this->hWnd,DDSCL_SETFOCUSWINDOW),
				1,
				FAILED(res)
			);
		}

		// now set the flags for the coop level

		DWORD flags = 0;

		// allow these flags nomatter what
		flags |= DDSCL_FULLSCREEN;
		flags |= DDSCL_EXCLUSIVE;
		flags |= DDSCL_ALLOWREBOOT;

		if(this->mode_bpp < 0)
		{
			flags |= DDSCL_ALLOWMODEX;
		}

		if(MULTIMON_NOTFIRSTDISPLAYCREATED == CurrentMonitor().multimon_type)
		{
			flags |= DDSCL_SETFOCUSWINDOW;
			flags |= DDSCL_CREATEDEVICEWINDOW;
		}

		// call the actual set coop level function until we have enough memory
		Memory_Alloc_Function(
			res = CurrentMonitor().dd->SetCooperativeLevel(this->hWnd,flags),
			1,
			FAILED(res)
		);

		flags = 0;

		if(this->mode_bpp < 0)
		{
			if(MODEBPP_13h == this->mode_bpp)
			{
				flags |= DDSDM_STANDARDVGAMODE;
			}
			this->mode_bpp = MODEX_BPP;
		}

		res = CurrentMonitor().dd->SetDisplayMode(
			CurrentMonitor().mode_width,
			CurrentMonitor().mode_height,
			this->mode_bpp,
			this->mode_refresh,
			flags
		);

		// check for failure
		if(FAILED(res))
		{
			return 1;
		}

		// allocate array for buffers
		b = new LPDIRECTDRAWSURFACE7[num_buffers];

		if(NULL == b)
		{
			MessageBox(this->hWnd,CGRAPHCERTERR_OUTOFMEMORY,NULL,MB_ICONSTOP);
			exit(1);
		}

		// create the primary surface
		memset((void *)&surf,0,sizeof(surf));
		surf.dwSize = sizeof(surf);

		surf.ddsCaps.dwCaps |= DDSCAPS_PRIMARYSURFACE;
		
		if(num_buffers > 1)
		{
			surf.ddsCaps.dwCaps |= DDSCAPS_COMPLEX;
			surf.ddsCaps.dwCaps |= DDSCAPS_FLIP;
		}
						
		if(mode_bpp <= PALETTIZED_COLOR_256C) // palettized
		{
			surf.ddsCaps.dwCaps |= DDSCAPS_PALETTE;
		}

		surf.dwFlags = DDSD_CAPS;

		if(num_buffers > 1)
		{
			surf.dwBackBufferCount = num_buffers - 1;
			surf.dwFlags |= DDSD_BACKBUFFERCOUNT;
		}
			
		Memory_Alloc_Function(
			res = CurrentMonitor().dd->CreateSurface(&surf,&(b[0]),NULL),
			CurrentMonitor().mode_width * CurrentMonitor().mode_height * this->mode_bpp,
			FAILED(res)
		);

		if(num_buffers > 1)
		{
			current_buffer = 1; // use current buffer to keep count

			// create the secondary surfaces
			while(FAILED(b[0]->EnumAttachedSurfaces(this,EnumSurfacesCallback2)))
			{
				SurfaceLost(b[0]);
			}

			current_buffer = 1; // reset count
		}
		else
		{
			current_buffer = 0;
		}

		// allocate memory for clip list
		cr =
			LPRGNDATA(new unsigned char[sizeof(RGNDATAHEADER)+sizeof(RECT)]);

		if(NULL == cr)
		{
			MessageBox(NULL,CGRAPHCERTERR_OUTOFMEMORY,NULL,MB_ICONSTOP); // check for error
			exit(1);
		}

		// set up the full-screen
		scr.left = scr.top = 0;
		scr.right = mode_bpp ?
			CurrentMonitor().mode_width : GetSystemMetrics(SM_CXSCREEN);
		scr.bottom = mode_bpp ?
			CurrentMonitor().mode_height : GetSystemMetrics(SM_CXSCREEN);	

		// copy scr to two places
		memcpy((void *)cr->Buffer,(const void *)&scr,sizeof(RECT));
		memcpy((void *)&cr->rdh.rcBound,(const void *)&scr,sizeof(RECT));

		cr->rdh.dwSize = sizeof(RGNDATAHEADER);
		cr->rdh.iType = RDH_RECTANGLES;
		cr->rdh.nCount = 1;
		cr->rdh.nRgnSize = sizeof(RECT);

		// set clipper clip list
		Memory_Alloc_Function(
			res = c->SetClipList(cr,0),
			sizeof(RECT) + sizeof(RGNDATAHEADER),
			FAILED(res)
		);

		// done with the region data
		delete (unsigned char *)cr;
		cr = NULL; // in case we try to delete it again

		// attach clipper to current_buffer
		b[current_buffer]->SetClipper(c);

		// setup the pf data structure
		memset((void *)&pf,0,sizeof(pf));
		pf.dwSize = sizeof(pf);

		// get the pixel format data
		b[current_buffer]->GetPixelFormat(&pf);

		// copy
		CurrentMonitor().a_bits = NumberOfBitsOn(pf.dwRGBAlphaBitMask);
		CurrentMonitor().ar_bits =
			CurrentMonitor().a_bits +
			NumberOfBitsOn(pf.dwRBitMask);
		CurrentMonitor().arg_bits =
			CurrentMonitor().ar_bits +
			NumberOfBitsOn(pf.dwGBitMask);

		is_certified = true;  // at last, we've finished

		return 0;
	}

	void CGraphics::Uncertify() {
		if(!is_certified) return;
		is_certified = false;

		// leave graphics
		b[current_buffer]->SetClipper(NULL);
		c->SetClipList(NULL,0);
		b[0]->Release(); // thereby releasing all of them
		delete [] b;
		if(mode_bpp) CurrentMonitor().dd->RestoreDisplayMode();
		CurrentMonitor().dd->SetCooperativeLevel(hWnd,DDSCL_NORMAL);
	}

	void CGraphics::Transition(int transition) {
		// perform a screen transition
		// surface description and pointer shortcuts:
		DDSURFACEDESC2 ddraw_sur_desc;
		USHORT **surface = (USHORT **)&(ddraw_sur_desc.lpSurface);
		LONG *pitch = &(ddraw_sur_desc.lPitch);
		LPDIRECTDRAWSURFACE7 front_buffer = b[0];
		LPDIRECTDRAWSURFACE7 back_buffer = b[current_buffer];
		CTimer timer;
				
		// blit effects:
		DDBLTFX ddraw_blit_fx; 
	
		// a rectangle:
		RECT rect;

		RECT jeopardy[48]; // jeopardy rectangles

		int worm_progress[16],worm_speed[16];
		int i,j; // loop variables

		assert(num_buffers > 1 && is_certified); // need double buffering for these effects

		// these values are always the same:
	
		// clear out the blit fx and set the size field 
		memset((void *)&ddraw_blit_fx,0,sizeof(ddraw_blit_fx));
		ddraw_blit_fx.dwSize = sizeof(ddraw_blit_fx);

		// seed random number generator
		srand((unsigned int)clock());

		switch(transition) {
		case TRANSITION_HORIZONTAL_SWIPE:
			// set the constant rect properties:
			rect.top = 0;
			rect.bottom = CurrentMonitor().mode_height;

			for(
				rect.right = 2;
				rect.right <= CurrentMonitor().mode_width;
				rect.right+=2) {
				timer.Restart();
				rect.left = rect.right - 2;
			
				// ready to blt to surface
				front_buffer->Blt(
					&rect,      // ptr to dest rectangle
					back_buffer,       // ptr to source surface, back buffer
					&rect,       // ptr to source rectangle, NA
					DDBLT_WAIT,   // wait and fill                   
					&ddraw_blit_fx);  // ptr to DDBLTFX structure

				do
				{
				}
				while(timer.SecondsSinceLastRestart() < 1.0f/float(this->CurrentMonitor().mode_width>>1));
			}
			break;
		case TRANSITION_BLINDS:
			// set the constant rect values:
			rect.left = 0;
			rect.right = CurrentMonitor().mode_width;;

			// there are 6 blinds
			for(i = 1; i <= CurrentMonitor().mode_width / 6; i++) {
				timer.Restart();

				for(int j = 0; j < 6; j++) { 
					// must loop the blinds here:
					rect.top = j * (CurrentMonitor().mode_width / 6);
					rect.bottom = rect.top+i;
				
					// ready to blt to surface
					front_buffer->Blt(
						&rect,      // ptr to dest rectangle
						back_buffer, // ptr to source surface, back buffer
						&rect,       // ptr to source rectangle
						DDBLT_WAIT,   // wait and fill                   
						&ddraw_blit_fx);  // ptr to DDBLTFX structure
				}
			
				do
				{
				}
				while(timer.SecondsSinceLastRestart() < 12.0f/float(this->CurrentMonitor().mode_height));
			}
			break;
		case TRANSITION_JEOPARDY_BLOCKS:
			// puts a bunch of blocks on the screen
			//  (8 columns x 6 rows)

			// create an organize list of rectangles:
			for(i = 0; i < 48; i++) {
				jeopardy[i].left = (i & 8) * (CurrentMonitor().mode_width>>3);
				jeopardy[i].right = jeopardy[i].left + (CurrentMonitor().mode_width>>3);
				jeopardy[i].top = (i >> 3) * (CurrentMonitor().mode_height/6);
				jeopardy[i].bottom = (jeopardy[i].top) + (CurrentMonitor().mode_height/6);
			}

			// now let's mess that list up by shuffling its
			//  contents!
			for(i = 0; i < 1000; i++) {
				// now we swap two random jeopardy squares:
				RECT* s1; // first square to swap
				RECT* s2; // other square to swap
				RECT temp; // helps with swapping
	
				s1 = &(jeopardy[rand()%48]); // get some random squares
				do
					s2 = &(jeopardy[rand()%48]);
				while(s2 == s1);

				// swap s1 and s2:
				memcpy((void *)&temp,(const void *)s1,sizeof(RECT));
				memcpy((void *)s1,(const void *)s2,sizeof(RECT));
				memcpy((void *)s2,(const void *)&temp,sizeof(RECT));
			}

			// and at last, now we blit the black blocks to the screen:
			for(i = 0; i < 48; i++) {
				timer.Restart();

				// ready to blt to surface
				front_buffer->Blt(
					&(jeopardy[i]),      // ptr to dest rectangle
					back_buffer, // ptr to source surface, the back buffer!
					&(jeopardy[i]),       // ptr to source rectangle
					DDBLT_WAIT,   // wait and fill
					&ddraw_blit_fx);  // ptr to DDBLTFX structure
			
				do
				{
				}
				while(timer.SecondsSinceLastRestart() < 1.0f/2000.0f);
			}
			break;
		case TRANSITION_MELTING:
			// set all the worm progress to zero
			memset((void *)worm_progress,0,sizeof(int) * 16);
		
			for(i = 0; i < 16; i++)
				worm_speed[i] = int(
					float((rand()%5)+4) * ((float)CurrentMonitor().mode_height/200.0)
				);

			// give them all loops to get to the bottom
			for(i = 0; i < 60; i++) {
				timer.Restart();

				// move the worms and draw:
				for(j = 0;j < 16;j++) {
					rect.top = worm_progress[j];
					rect.left = j;
					rect.right = rect.left + (CurrentMonitor().mode_width >> 4);
					worm_progress[j] += worm_speed[j];
					if(worm_progress[j] > CurrentMonitor().mode_height)
						worm_progress[j] = CurrentMonitor().mode_height;
					rect.bottom = worm_progress[j];	
					front_buffer->Blt(
						&rect, // the rectangle
						back_buffer, // use back buffer
						&rect, // no source rect, use whole screen
						DDBLT_WAIT, // flags
						&ddraw_blit_fx);
				}
				// wait a while:
				do
				{
				}
				while(timer.SecondsSinceLastRestart() < 1.0f/30.0f);
			}
			break;
		default:
			assert(false);
		}
	}

	const CScreenArea& CGraphics::cTargetScreenArea() const {
		assert(true == this->is_certified);
		return current_area;
	}

	void CGraphics::Flip() {
		assert(true == this->is_certified);
		DWORD flags;

		flags = DDFLIP_WAIT;
		
		while(FAILED(b[0]->Flip(NULL,DDFLIP_WAIT)))
		{
			for(int i = 0; i < this->num_buffers; i++)
			{
				if(DDERR_SURFACELOST == b[i]->IsLost())
				{
					SurfaceLost(b[i]);
				}
			}
		}
	}

	const GUID *CGraphics::GetDirectDrawDriver() const {
		return directdraw_driver;
	}

	int CGraphics::GetModeBpp() const {
		return mode_bpp < 0 ? 8 : mode_bpp;
	}

	int CGraphics::GetNumBuffers() const {
		return num_buffers;
	}

	int CGraphics::GetRefreshRate() const {
		return mode_refresh;
	}

	int CGraphics::GetTargetBuffer() const {
		assert(is_certified);
		return current_buffer;
	}

	HWND CGraphics::GetWindow() const {
		return hWnd;
	}

	bool CGraphics::ModeIs13h() const {
		return (mode_bpp == -1) ? true : false;
	}

	bool CGraphics::ModeIsX() const {
		return (mode_bpp == -2) ? true : false;
	}

	void CGraphics::Put(CColorMap& bob,bool use_transparency) {
		assert(true == this->OnMyMonitor());
		assert(true == bob.OnMyMonitor());

		while(FAILED(b[current_buffer]->Blt(
			&current_area.Area(),
			bob.data,
			NULL,
			use_transparency ? DDBLT_KEYSRC : 0,
			NULL)))
		{
			SurfaceLost(b[current_buffer]);
		}
	}

	void CGraphics::Rectangle(const CColor& c) {
		assert(is_certified && OnMyMonitor());
		DDBLTFX fx;

		// init fx struct
		memset((void *)&fx,0,sizeof(fx));
		fx.dwSize = sizeof(fx);

		fx.dwFillColor = c.Color32();

		while(FAILED(b[current_buffer]->Blt(
			&current_area.Area(),
			NULL,
			NULL,
			DDBLT_COLORFILL,
			&fx)))
		{
			SurfaceLost(b[current_buffer]);
		}
	}

	void CGraphics::SetMode(int mode_width,int mode_height,int mode_bpp_) {
		assert(!is_certified);

		mode_bpp = mode_bpp_;
		CurrentMonitor().mode_width = mode_width;
		CurrentMonitor().mode_height = mode_height;
	}

	void CGraphics::SetMode(int mode) {
		assert(!is_certified);

		mode_bpp = (mode == MODE13h) ? -1 : -2;
		CurrentMonitor().mode_width = 320;
		CurrentMonitor().mode_width =
			(mode == MODEX_320x240) ? 240 : 200;
	}

	void CGraphics::SetNumBuffers(int num_buffers_) {
		assert(!is_certified);

		num_buffers = num_buffers_;
	}

	void CGraphics::SetRefreshRate(int mode_refresh_) {
		assert(!is_certified);

		mode_refresh = mode_refresh_;
	}

	void CGraphics::SetTargetBuffer(int buffer_index) {
		assert(is_certified);

		// unattach the clipper from the former target
		while(FAILED(b[current_buffer]->SetClipper(NULL))) 
		{
			SurfaceLost(b[current_buffer]);
		}
			
		current_buffer = buffer_index; // point towards the new target

		while(FAILED(b[current_buffer]->SetClipper(c)))
		{
			SurfaceLost(b[current_buffer]);
		}
	}

	void CGraphics::SetWindow(HWND hWnd_) {
		assert(false == this->is_certified);

		hWnd = hWnd_;
	}

	CScreenArea& CGraphics::TargetScreenArea() {
		assert(true == this->is_certified);

		return current_area;
	}
}
