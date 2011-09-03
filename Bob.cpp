#include "StdAfx.h"
#include "Certifiable.h"
#include "Color.h"
#include "ColorNP.h"
#include "Color256.h"
#include "Bob.h"
#include "CompactMap.h"
#include "Graphics.h"
#include "Logger.h"

CBob::CBob() : data(NULL) {WriteLog("CBob generic constructor initiated for Bob at %8x" LogArg((DWORD)this));}

CBob::CBob(LPDIRECTDRAWSURFACE2 parent) : data(NULL)
{
  WriteLog("CBob duplication of surface constructor called for Bob at %8x" LogArg((DWORD)this));
  this->Create(parent);
}

CBob::~CBob()
{
  //WriteLog("CBob Destructor called for Bob at %8x",(DWORD)this);
  if(NULL != this->data)
    {
      //WriteLog("Surface for Bob must be released");
      //TryAndReport(this->data->Release());
      this->data->Release();
    }
}

int CBob::GetWidth()
{
  WriteLog("CBob::GetWidth called for Bob at %8x" LogArg((DWORD)this));
  // this is not const member because we need to make
  //  a call to the DDSURFACE interface
  int w;
  int h;
  this->GetSize(w,h);
  WriteLog("CBob::GetWidth: we found out that the width was %d, but also the height was %d" LogArg(w) LogArg(h) );
  return w;
}

int CBob::GetHeight()
{
  WriteLog("CBob::GetHeight called for Bob at %8x" LogArg((DWORD)this));
  // this is not const member because we need to make 
  //  a call to the DDSRUFACE interface
  int w;
  int h;
  this->GetSize(w,h);
  WriteLog("CBob::GetHeight: we found out that the height was but also that the width was %d" LogArg(h) LogArg(w));
  return h;
}

void CBob::GetSize(int& width,int& height)
{
  WriteLog("CBob::GetSize called for Bob at %8x" LogArg((DWORD)this));
  width = entire.right;
  height = entire.bottom;

  WriteLog("CBob::GetSize: we found out the surface was %dx%d" LogArg(width) LogArg(height));
}

void CBob::Create(LPDIRECTDRAWSURFACE2 parent)
{
  WriteLog("CBob::Create called; about to duplicate a surface at %8x for Bob at %8x" LogArg((DWORD)parent) LogArg((DWORD)this));
  /*assert(NULL == this->data);
  HRESULT res;
  LPDIRECTDRAWSURFACE version1_of_data;
  LPDIRECTDRAWSURFACE version1_of_parent;

  WriteLog("get old interface of parent...");
  MemoryAllocFunction(res = TryAndReport(parent->QueryInterface(IID_IDirectDrawSurface,(void **)&version1_of_parent)),
		      sizeof(int), FAILED(res));

  WriteLog("duplicate the surface...");
  MemoryAllocFunction(res = TryAndReport(CGraphics::DirectDraw()->DuplicateSurface(version1_of_parent,&version1_of_data)),
		      sizeof(IDirectDrawSurface), FAILED(res));

  WriteLog("get the new version of the result...");
  MemoryAllocFunction(res = TryAndReport(version1_of_data->QueryInterface(IID_IDirectDrawSurface2,(void **)&this->data)),
		      sizeof(IDirectDrawSurface2), FAILED(res));
  this->data = parent;*/

  (data = parent)->AddRef();

  DDSURFACEDESC ddsd;

  memset((void *)&ddsd,0,sizeof(ddsd));
  ddsd.dwSize = sizeof(ddsd);

  TryAndReport(this->data->GetSurfaceDesc(&ddsd));

  entire.left = entire.top = 0;
  entire.right = ddsd.dwWidth;
  entire.bottom = ddsd.dwHeight;

  WriteLog("CBob::Create finished");
}

void CBob::Destroy()
{
  WriteLog("CBob::Destroy called for Bob at memory address %8x" LogArg((DWORD)this));
  assert(NULL != this->data);

  TryAndReport(this->data->Release());

  this->data = NULL;
  WriteLog("CBob::Destroy all done!");
}

CBob::CBob(int w,int h,DWORD orFlags,DWORD andFlags) : data(NULL)
{
  WriteLog("CBob blank surface constructor called for Bob at address %8x" LogArg((DWORD)this));
  this->Create(w,h,orFlags,andFlags);
}

CBob::CBob(HBITMAP bmp,DWORD orFlags,DWORD andFlags) : data(NULL)
{
  WriteLog("CBob load bitmap constructor called for Bob at addrees %8x" LogArg((DWORD)this));
  WriteLog("This function loads a bitmap into the surface from an HBITMAP object");
  BITMAP bmp_info;

  TryAndReport(GetObject((HGDIOBJ)bmp,sizeof(BITMAP),(void *)&bmp_info));

  WriteLog("Result of GetObject; bmp_info struct contains this data:");
  WriteLog("bmType:       %d" LogArg(bmp_info.bmType));
  WriteLog("bmWidth:      %d" LogArg(bmp_info.bmWidth));
  WriteLog("bmHeight:     %d" LogArg(bmp_info.bmHeight));
  WriteLog("bmWidthBytes: %d" LogArg(bmp_info.bmWidthBytes));
  WriteLog("bmPlanes:     %d" LogArg((int)bmp_info.bmPlanes));
  WriteLog("bmBitsPixel:  %d" LogArg((int)bmp_info.bmBitsPixel));
  WriteLog("bmBits:       %8x" LogArg((DWORD)bmp_info.bmBits));
		
  // create the surface
  this->Create(bmp_info.bmWidth,bmp_info.bmHeight,orFlags,andFlags);

  // extract the data
  this->Extract(bmp,0,0);

  WriteLog("CBob load Bitmap constructor finished");
}


void CBob::Create(int w,int h,DWORD orFlags,DWORD andFlags)
{
  WriteLog("CBob::Create called to make a surface of size %dx%d for Bob at %8d . . . about to call CreateSurface a few times until we succeed" LogArg(w) LogArg(h) LogArg((DWORD)this));
  DDSURFACEDESC d;
  HRESULT res;
		
  assert(NULL == this->data);

  memset((void *)&d,0,sizeof(d)); // zero out d

  d.dwSize = sizeof(d); // set the size member

  // set the dimensions of the surface description
  d.dwWidth = w;
  d.dwHeight = h;

  // set surface to offscreen plain in video memory
  d.ddsCaps.dwCaps |= DDSCAPS_OFFSCREENPLAIN;

  // apply default memory options
  d.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;

  // apply customization (that's right, we do it after the memory options)
  //  this will allow a lot of control over where to put it into memory
  //  by default, first we try local video memory, then non-local, and then
  //  system memory
  d.ddsCaps.dwCaps |= orFlags;
  d.ddsCaps.dwCaps &= andFlags;

  // now see if the caps specify both system and video
  //  memory.  If so, the caller probably wants system memory.
  //  If it specifies neither, the caller probably wants system
  //   memory
  if(!(d.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY) && !(d.ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY))
    {
      // neither flag was specified, so let's turn on system
      //  memory (the user probably disabled video memory in
      //  the "and" caps struct)
      d.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
    }
  else if(d.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY && d.ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY)
    {
      // both memory flags were specified, so let's turn on system
      //  memory (the user probably enabled it in the "or" caps
      //  struct, but forgot to disable the video memory  in the
      //  "and" caps struct
      // turn on system memory by turning off the vidoe memory
      //  flag
      d.ddsCaps.dwCaps &= ~(DDSCAPS_VIDEOMEMORY);
    }

  // set the flags of the direct draw surface desc
  //  we have specified the caps, width, and height
  d.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;

  LPDIRECTDRAWSURFACE version1_of_data;
		
  // check if we are using special memory options
  if(!(andFlags & DDSCAPS_VIDEOMEMORY))
    {
      // the caller was trying to disable video memory
      //  which was not what we were planning on
      if(SUCCEEDED(TryAndReport(CGraphics::DirectDraw()->CreateSurface(&d,&version1_of_data,NULL))))
	{
	  goto surface_create_success;
	}

      // stupid caller!  disabling video memory when a surface
      //  could not be created there!

      // if we ended up here, then the caller's memory plans did not work . . .
      // so make our own, and continue functioning normally
      d.ddsCaps.dwCaps &= ~DDSCAPS_SYSTEMMEMORY;
      d.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
    }

  // try to create the surface
  if(FAILED(TryAndReport(CGraphics::DirectDraw()->CreateSurface(&d,&version1_of_data,NULL)))) {

    // if we fail, try to allocate in system memory
    d.ddsCaps.dwCaps &=
      ~DDSCAPS_VIDEOMEMORY; 
    d.ddsCaps.dwCaps |=
      DDSCAPS_SYSTEMMEMORY;
    MemoryAllocFunction(
			res = TryAndReport(CGraphics::DirectDraw()->CreateSurface(&d,&version1_of_data,NULL)),
			w * h * CGraphics::BytesPerPixel(),
			FAILED(res)
			);
  }

 surface_create_success:
  // query the old interface for a new interface
  MemoryAllocFunction(
		      res = TryAndReport(version1_of_data->QueryInterface(IID_IDirectDrawSurface2,(void **)&this->data)),
		      sizeof(IDirectDrawSurface2),
		      FAILED(res)
		      );

  // release the old interface
  TryAndReport(version1_of_data->Release());

  // set the transparent color to zero
  //  if this surface is palettized, this means
  //  the transparent color will be the first on the
  //  palette.  This is usually black.
  //  If this surface is not palettized, this
  //   means the transparent color will be black
  //   for sure
  this->SetTransparentColorInternal(0);

  entire.left = entire.top = 0;
  entire.right = w;
  entire.bottom = h;

  WriteLog("Finished CBob::Create, surface cap flags = %8d" LogArg(d.ddsCaps.dwCaps));
}

void CBob::Extract(HBITMAP bmp,int x,int y)
{
  WriteLog("CBob::Extract called for Bob at memory address %8d" LogArg((DWORD)this));
  assert(NULL != this->data);
				
  WriteLog("Calling CBob::GetSize to find our dimensions...");
  int width;
  int height;
  this->GetSize(width,height);

  WriteLog("About to get the DC of our surface");
  HDC me;
  if(FAILED(TryAndReport(this->data->GetDC(&me))))
    {
      WriteLog("GetDC failed; CBob::Extract terminating...");
      return; 
    }

  WriteLog("About to make compatible DC and select a BMP into it");
  HDC bit_dc= TryAndReport(CreateCompatibleDC(me));
  HGDIOBJ old_bmp = TryAndReport(SelectObject(bit_dc,bmp));

  HPALETTE prev_pal;

  if(1 == CGraphics::BytesPerPixel())
    {
      WriteLog("We are in 8-bit color mode, selecting special palette into destinacion DC");
      prev_pal = SelectPalette(me,CColor256::GetGDIPalette(),FALSE);
    }

  // copy
  TryAndReport(BitBlt(me,0,0,width,height,bit_dc,x,y,SRCCOPY));

  if(1 == CGraphics::BytesPerPixel())
    {
      WriteLog("Restoring old palette of destination DC");
      SelectPalette(me,prev_pal,FALSE);
    }

  // release our dc, because it is not needed and has
  //  succeeded in storing the app data on the surface
  WriteLog("Getting rid of destination DC");
  TryAndReport(data->ReleaseDC(me));

  WriteLog("Restoring old bitmap to source DC");
  TryAndReport(SelectObject(bit_dc,old_bmp));

  WriteLog("Getting rid of source DC");
  TryAndReport(DeleteDC(bit_dc));
}

void CBob::SetTransparentColor(BYTE c)
{
  WriteLog("CBob::SetTransparentColor called for Bob at memory address %8d to set to color %2x" LogArg((DWORD)this) LogArg((int)c));
  assert(NULL != this->data);
  this->SetTransparentColorInternal((DWORD)c);
}

void CBob::SetTransparentColor(const CColorNP& c)
{
  WriteLog("CBob::SetTransparentColor called for Bob at memory address %8d to set color to %8d" LogArg((DWORD)this) LogArg((DWORD)c.Color32b()));
  assert(NULL != this->data);
  this->SetTransparentColorInternal(c.Color32b());
}

void CBob::SetTransparentColorInternal(DWORD c)
{
  WriteLog("CBob::SetTransparentColorInternal called for Bob at memory address %8x to set color to %8x" LogArg((DWORD)this) LogArg(c));
  DDCOLORKEY k = {c,c};
  TryAndReport(this->data->SetColorKey(DDCKEY_SRCBLT,&k));
}

bool CBob::IsCreated() const
{
  WriteLog("CBob::IsCreated called for Bob at memory address %8d with a surface"
	   " at memory address %8d" LogArg((DWORD)this)
	   LogArg((DWORD)this->data));
  return bool(NULL != this->data);
}

