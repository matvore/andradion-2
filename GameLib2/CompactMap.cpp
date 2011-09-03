// CompactMap.cpp: implementation of the CCompactMap class.
//
//////////////////////////////////////////////////////////////////////

#include "Certifiable.h"
#include "Color.h"
#include "Color256.h"
#include "ColorNP.h"
#include "CompactMap.h"
#include "LazyErrHandling.h"
#include "Graphics.h"
#include "Logger.h"

// Comment the next three lines to allow this module to log
#define WriteLog(x)
#undef LogArg
#define LogArg(x)
#undef TryAndReport
#define TryAndReport(x) x

using std::map;
using std::pair;


namespace NGameLib2
{
#ifdef BORLAND
#define BINLINE
	using std::min;
	using std::max;
#define __min min
#define __max max
#else
#define BINLINE inline
#endif

	// putting individual rows:
	BINLINE void PutCompactMapRowNoLeftRightClip
	(
		BYTE *lpSurface,VCTR_RUN& row,int start_x
	)
	{
		for(VCTR_RUN::iterator run_i = row.begin(); run_i != row.end(); run_i++)
		{
		  WriteLog("Putting run %8x" LogArg((DWORD)run_i));
			memcpy(
				(void *)(lpSurface+start_x+run_i->first),
				(const void *)run_i->second.begin(),
				run_i->second.size()
			);
		}
	}

	BINLINE void PutCompactMapRowRightClipOnly
	(
		BYTE *lpSurface,VCTR_RUN& row,int start_x,int max_x
	)
	{
		// clip on right
		for(VCTR_RUN::iterator run_i = row.begin(); run_i != row.end(); run_i++)
		{
			int x = start_x + run_i->first;
			int bytes_to_copy = run_i->second.size();
			if(x + bytes_to_copy > max_x)
			{
				bytes_to_copy = max_x-x;
				if(bytes_to_copy <= 0)
				{
					break;
				}
			}
			memcpy((void *)(lpSurface+x),(const void *)run_i->second.begin(),bytes_to_copy);
		}
	}

	BINLINE void PutCompactMapRowLeftClipOnly
	(
		BYTE *lpSurface,VCTR_RUN& row,int start_x,int min_x
	)
	{
		// clip on left
		VCTR_RUN::const_iterator run_i;
		for(run_i = row.begin(); run_i != row.end(); run_i++)
		{
			int x = start_x + run_i->first;
			int size = run_i->second.size();
			if(x + size > min_x)
			{
				int first_byte_to_copy = min_x - x;
				if(first_byte_to_copy >= 0)
				{
					memcpy
					(
						(void *)(lpSurface+min_x),
						(const void *)(run_i->second.begin()+first_byte_to_copy),
						size - first_byte_to_copy
					);
					run_i++;
				}
				break;
			}
		}

		// no clipping for a while
		for(;run_i != row.end(); run_i++)
		{
			memcpy(
				(void *)(lpSurface+start_x+run_i->first),
				(const void *)run_i->second.begin(),
				run_i->second.size()
			);
		}
	}

	BINLINE void PutCompactMapRowLeftRightClip
	(
		BYTE *lpSurface,VCTR_RUN& row,int start_x,int min_x,int max_x
	)
	{
		// clip on left
		VCTR_RUN::const_iterator run_i;
		for(run_i = row.begin(); run_i != row.end(); run_i++)
		{
			int x = start_x + run_i->first;
			if(x + (int)run_i->second.size() > min_x)
			{
				int first_byte_to_copy;
				int surface_offset;
				if(min_x <= x)
				{
					first_byte_to_copy = 0;
					surface_offset = x;
				}
				else
				{
					first_byte_to_copy = min_x - x;
					surface_offset = min_x;
				}
				int bytes_to_copy = run_i->second.size() - first_byte_to_copy;
				if(surface_offset + bytes_to_copy > max_x)
				{
					bytes_to_copy = max_x-surface_offset;
				}
				if(bytes_to_copy > 0)
				{
					memcpy
					(
						(void *)(lpSurface+surface_offset),
						(const void *)(run_i->second.begin()+first_byte_to_copy),
						bytes_to_copy
					);
					break;
				}
			}
		}
		
		// clip on right
		for(run_i++; run_i != row.end(); run_i++)
		{
			int x = start_x + run_i->first;
			int bytes_to_copy = run_i->second.size();
			if(x + bytes_to_copy > max_x)
			{
				bytes_to_copy = max_x-x;
				if(bytes_to_copy <= 0)
				{
					break;
				}
			}
			memcpy((void *)(lpSurface+x),(const void *)run_i->second.begin(),bytes_to_copy);
		}
	}

	// putting entire maps:
	BINLINE void PutCompactMapStep2of2NoLeftRightClip
	(
		BYTE *lpSurface,long pitch,CCompactMap& bob,int start_y,int end_y,int start_x
	)
	{
		for(int y = start_y;y < end_y; y++)
		{
		  WriteLog("Putting y of %d" LogArg(y));
			PutCompactMapRowNoLeftRightClip(lpSurface,bob.left_over[y],start_x);
			lpSurface += pitch;
		}
	}

	BINLINE void PutCompactMapStep2of2RightClipOnly
	(
		BYTE *lpSurface,long pitch,CCompactMap& bob,int start_y,int end_y,int start_x,int max_x
	)
	{
		for(int y = start_y; y < end_y; y++)
		{
			if(bob.min_max_x_row_offsets[y].second +start_x < max_x)
			{
				PutCompactMapRowNoLeftRightClip(lpSurface,bob.left_over[y],start_x);
			}
			else
			{
				PutCompactMapRowRightClipOnly(lpSurface,bob.left_over[y],start_x,max_x);
			}
			lpSurface += pitch;
		}
	}

	BINLINE void PutCompactMapStep2of2LeftClipOnly
	(
		BYTE *lpSurface,long pitch,CCompactMap& bob,int start_y,int end_y,int start_x,int min_x
	)
	{
		for(int y = start_y; y < end_y; y++)
		{
			if(bob.min_max_x_row_offsets[y].first +start_x< min_x)
			{
				PutCompactMapRowLeftClipOnly(lpSurface,bob.left_over[y],start_x,min_x);
			}
			else
			{
				PutCompactMapRowNoLeftRightClip(lpSurface,bob.left_over[y],start_x);
			}
			lpSurface += pitch;
		}
	}

	BINLINE void PutCompactMapStep2of2LeftRightClip
	(
		BYTE *lpSurface,long pitch,CCompactMap& bob,int start_y,int end_y,int start_x,int min_x,int max_x
	)
	{
		for(int y = start_y; y < end_y; y++)
		{
			if(bob.min_max_x_row_offsets[y].first +start_x>= min_x)
			{
				// no left clipping
				if(bob.min_max_x_row_offsets[y].second +start_x< max_x)
				{
					// no right clipping or left clipping
					PutCompactMapRowNoLeftRightClip(lpSurface,bob.left_over[y],start_x);
				}
				else
				{
					// right clipping, but no left clipping
					PutCompactMapRowRightClipOnly(lpSurface,bob.left_over[y],start_x,max_x);
				}
			}
			else if(bob.min_max_x_row_offsets[y].second +start_x< max_x)
			{
				// no right clipping, but left clipping
				PutCompactMapRowLeftClipOnly(lpSurface,bob.left_over[y],start_x,min_x);
			}
			else
			{
				// full clipping
				PutCompactMapRowLeftRightClip(lpSurface,bob.left_over[y],start_x,min_x,max_x);
			}
			lpSurface += pitch;
		}
	}

	void CCompactMap::RenderStep1(IDirectDrawSurface2 *target_buffer,int current_area_left,int current_area_top,bool async)
	{
		assert(true == this->Certified() || !"Tried to blit an uncertified CCompactMap");
		DWORD behavior_flags = (true == async) ? DDBLT_ASYNC : 0;

		// prepare a DDBLTFX structure
		DDBLTFX fx;
		fx.dwSize = sizeof(fx);
		
		// blit blocks
		VCTR_DWORD::iterator block_i;
		VCTR_RECTANGLEVECTOR::iterator area_i;
		behavior_flags |= DDBLT_COLORFILL;
		for
		(
			block_i = this->blocks.begin(),area_i = this->block_areas.begin();
			block_i != this->blocks.end() /*&& area_i != this->block_areas.end() */;
			block_i++, area_i++
		)
		{
			fx.dwFillColor = *block_i;

			VCTR_RECTANGLE::iterator rect_i;
			for(rect_i = area_i->begin();rect_i != area_i->end();rect_i++)
			{
				RECT t = *rect_i;
				t.left += current_area_left;
				t.right += current_area_left;
				t.top += current_area_top;
				t.bottom += current_area_top;
				
				while
				(
					DDERR_WASSTILLDRAWING ==
						target_buffer->Blt(&t,NULL,NULL,behavior_flags,&fx)
				);
			}
		}
		behavior_flags &= ~DDBLT_COLORFILL;

		// blit patterns
		VCTR_DIRECTDRAWSURFACE::iterator pattern_i;
		for
		(
			pattern_i = this->patterns.begin(),area_i = this->pattern_coors.begin();
			pattern_i != this->patterns.end() /*&& area_i != this->pattern_coors.end()*/;
			pattern_i++,area_i++
		)
		{
			VCTR_RECTANGLE::iterator rect_i;
			for(rect_i = area_i->begin(); rect_i != area_i->end(); rect_i++)
			{
				RECT t = *rect_i;
				t.left += current_area_left;
				t.right += current_area_left;
				t.top += current_area_top;
				t.bottom += current_area_top;
				
				while
				(
					DDERR_WASSTILLDRAWING ==
						target_buffer->Blt(&t,*pattern_i,NULL,behavior_flags,NULL)
				);
			}
		}
	}

	void CCompactMap::RenderStep1(IDirectDrawSurface2 *target_buffer,int current_area_left,int current_area_top,const RECT& simple_clipper_rect)
	{
		assert(true == this->Certified() || !"Tried to blit an uncertified compact map");
		// detach clipper
		LPDIRECTDRAWCLIPPER old;
		if(FAILED(target_buffer->GetClipper(&old)))
		{
			old = NULL;
		}
		else
		{
			target_buffer->SetClipper(NULL);
		}

		// prepare a DDBLTFX structure
		DDBLTFX fx;
		fx.dwSize = sizeof(fx);
		
		// create abbreviated local variables
		#define cl simple_clipper_rect.left // clipper left
		#define cr simple_clipper_rect.right // clipper right
		#define ct simple_clipper_rect.top // clipper top
		#define cb simple_clipper_rect.bottom // clipper bottom

		// blit blocks
		VCTR_DWORD::iterator block_i;
		VCTR_RECTANGLEVECTOR::iterator area_i;
		for
		(
			block_i = this->blocks.begin(),area_i = this->block_areas.begin();
			block_i != this->blocks.end() /*&& area_i != this->block_areas.end() */;
			block_i++, area_i++
		)
		{
			fx.dwFillColor = *block_i;

			VCTR_RECTANGLE::iterator rect_i;
			for(rect_i = area_i->begin();rect_i != area_i->end();rect_i++)
			{
				RECT t = *rect_i;
				t.left += current_area_left;
				t.right += current_area_left;
				t.top += current_area_top;
				t.bottom += current_area_top;

				if(t.left < cl)
				{
					t.left = cl;
				}
				if(t.right > cr)
				{
					t.right = cr;
				}
				if(t.top < ct)
				{
					t.top = ct;
				}
				if(t.bottom > cb)
				{
					t.bottom = cb;
				}

				if
				(
					// if this is false, then the second expression will not be evaluated:
					t.bottom > t.top && t.right > t.left 
					&&
					// this is not always executed:
					FAILED(target_buffer->Blt(&t,NULL,NULL,DDBLT_COLORFILL|DDBLT_ASYNC,&fx))
				)
				{
					while
					(
						DDERR_WASSTILLDRAWING ==
							target_buffer->Blt(&t,NULL,NULL,DDBLT_COLORFILL,&fx)
					);
				}
			}
		}

		// blit patterns
		VCTR_DIRECTDRAWSURFACE::iterator pattern_i;
		for
		(
			pattern_i = this->patterns.begin(),area_i = this->pattern_coors.begin();
			pattern_i != this->patterns.end() /*&& area_i != this->pattern_coors.end()*/;
			pattern_i++,area_i++
		)
		{
			int pw = (*area_i)[0].right - (*area_i)[0].left;
			int ph = (*area_i)[0].bottom - (*area_i)[0].top;
			VCTR_RECTANGLE::iterator rect_i;
			for(rect_i = area_i->begin(); rect_i != area_i->end(); rect_i++)
			{
				int x = rect_i->left+current_area_left;
				int y = rect_i->top+current_area_top;

				RECT s =
				{
					__max(0,(int)cl-x),
					__max(0,(int)ct-y),
					__min(pw,(int)cr-x),
					__min(ph,(int)cb-y)
				};

				if(x < cl)
				{
					x = cl;
				}
				if(y < ct)
				{
					y = ct;
				}

				while
				(
					DDERR_WASSTILLDRAWING ==
						target_buffer->BltFast(x,y,*pattern_i,&s,0)
				);
			}
		}
		
		// attach old clipper
		target_buffer->SetClipper(old);

		#undef cl
		#undef cr
		#undef cb
		#undef ct
	}

	void CCompactMap::RenderStep2(void *surface,int pitch,int x,int y,const RECT *simple_clipper_rect)
	{
	  WriteLog("CCompactMap::RenderStep2 called for compact map at address %8x" LogArg((DWORD)this));
		// we have to do complicated clipping manually

		// figure first and last row to blit in leftover
		// get a pointer into the surface, too
		BYTE *surf_8bit = (BYTE *)surface;
		int start_y;
		int end_y;
		if(NULL == simple_clipper_rect)
		{
		  WriteLog("No simple clipper rect was passed, using default calculations");
			start_y = 0;
			surf_8bit += pitch * y;
			end_y = this->left_over.size();
		}
		else
		{
		  WriteLog("Clipper rect supplied, performing clipping calculations");
			start_y = simple_clipper_rect->top-y;
			if(start_y < 0)
			{
				start_y = 0;
				surf_8bit += pitch * y;
			}
			else
			{
				surf_8bit += pitch * simple_clipper_rect->top;
			}
			end_y =
				__min
				(
					(int)this->left_over.size(),
					(int)simple_clipper_rect->bottom-y
				);
		}
		WriteLog("start_y: %d, end_y: %d, surface offset: %u" LogArg(start_y) LogArg(end_y) LogArg(surf_8bit - (BYTE *)surface));

		int bytes_per_pixel = CGraphics::BytesPerPixel();
		int start_x = x * bytes_per_pixel;
		WriteLog("bytes_per_pixel: %d, start_x: %d" LogArg(bytes_per_pixel) LogArg(start_x));
		if(NULL == simple_clipper_rect)
		{
		  WriteLog("Not clipping, calling NoLeftRightClip function");
			PutCompactMapStep2of2NoLeftRightClip(surf_8bit,pitch,*this,start_y,end_y,start_x);
			WriteLog("RenderStep2 returning");
			return;
		}
		int min_x = simple_clipper_rect->left * bytes_per_pixel;
		int max_x = simple_clipper_rect->right * bytes_per_pixel;
		if(this->min_max_x_offsets.first + start_x>= min_x)
		{
			// don't have to clip on left
			if(this->min_max_x_offsets.second +start_x < max_x)
			{
				// don't have to clip on right either!
				PutCompactMapStep2of2NoLeftRightClip(surf_8bit,pitch,*this,start_y,end_y,start_x);
			}
			else
			{
				// have to clip on right, but not left
				PutCompactMapStep2of2RightClipOnly(surf_8bit,pitch,*this,start_y,end_y,start_x,max_x);
			}
		}
		else if(this->min_max_x_offsets.second+start_x < max_x)
		{
			// have to clip on left, but not right
			PutCompactMapStep2of2LeftClipOnly(surf_8bit,pitch,*this,start_y,end_y,start_x,min_x);
		}
		else
		{
			// have to clip both ways
			PutCompactMapStep2of2LeftRightClip(surf_8bit,pitch,*this,start_y,end_y,start_x,min_x,max_x);
		}

	}



	CCompactMap::CCompactMap() : CCertifiable()	{WriteLog("Default CCompactMap constructor called for map at memory address %8x" LogArg((DWORD)this));}

	CCompactMap::~CCompactMap()
	{
		//WriteLog("Deconstructor called for CCompactMap at memory address %8x",(DWORD)this);
		if(this->Certified())
		{
			//WriteLog("Deconstructor must uncertify the object, because it is currently certified");
			this->Uncertify();
		}
		//WriteLog("Deconstructor for CCompactMap finished");
	}

	void CCompactMap::Uncertify()
	{
		WriteLog("CCompactMap::Uncertify called for CCompactMap at memory address %8x" LogArg((DWORD)this));
		if(!this->Certified())
		{
			WriteLog("CCompactMap::Uncertify returning because we are already uncertified");
			return;
		}

		// sanity checks
		assert(this->blocks.size() == this->block_areas.size());
		assert(this->patterns.size() == this->pattern_coors.size());

		WriteLog("Looping through all surfaces for patterns to release them");
		for
		(
			VCTR_DIRECTDRAWSURFACE::iterator iterate = this->patterns.begin();
			iterate != this->patterns.end();
			iterate++
		)
		{
			TryAndReport((**iterate).Release()); // release currently selected surface
		}

		WriteLog("Finished releasing all pattern surfaces");

		WriteLog("Resizing all of our vectors to zero, efectively freeing all the memory associated with them");
		//  this will free up some extra memory which could be very important
		//  if this compact map is destructed or recertified long after it is
		//  uncertified.  We could leave these lines out and the memory would
		//  still be released automatically by the vector destructor when the
		//  CCompactMap destructor is executed
		this->patterns.clear();
		this->pattern_coors.clear();
		this->blocks.clear();
		this->block_areas.clear();
		this->left_over.clear();

		WriteLog("Calling CCertifiable::Uncertify to finalize the decertification");
		CCertifiable::Uncertify();
		WriteLog("CCertifiable::Uncertify returning");
	}

	class datafile
	{
	public:
		datafile() {this->file = INVALID_HANDLE_VALUE;}
		~datafile() {if(INVALID_HANDLE_VALUE != this->file) this->close();}

		void get(DWORD& x,bool as_rgb_triple) 
		{
			if(NULL != res_data)
			{
				// take care of the resource scenario
				if(true == as_rgb_triple)
				{
					int c_data = *((WORD *)this->res_ptr);
					// get r,g,b individually
					int r = (c_data >> 11) << 3;
					int g = ((c_data >> 5) & 63) << 2;
					int b = (c_data & 31) << 3;
					x = (DWORD)RGB(r,g,b);
				}
				else
				{
					x = *((WORD *)this->res_ptr);
				}

				this->res_ptr += sizeof(WORD);

				return; // already done
			}

			// read from file . . .
			DWORD read;
			WORD data;
			ReadFile(this->file,(void *)&data,sizeof(WORD),&read,NULL);

			if(true == as_rgb_triple)
			{
				x = (DWORD)RGB((data >> 11) << 3,((data >> 5) & 127) << 2,(data & 63) << 3);
			}
			else
			{
				x = (DWORD)data;
			}
		}

		void open(const TCHAR *res_name,const TCHAR *res_type,HMODULE res_mod,WORD res_lang);

		void open(const TCHAR *fn) 
		{
			WriteLog("datafile::open (as file) called for data file at memory address %8x; all we have to do is call CreateFile:" LogArg((DWORD)this));
			this->file = TryAndReport(CreateFile(fn,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL));
			this->res_data = NULL;
		}

		bool fail()
		{
			return bool(INVALID_HANDLE_VALUE == this->file && NULL == this->res_data);
		}

		void close() 
		{
			WriteLog("datafile::close called for data file at memory address %8x" LogArg((DWORD)this));
			if(true == this->fail())
			{
				WriteLog("no action needed because we never succeeded to open it!");
				return;
			}

			if(NULL == this->res_data)
			{
				TryAndReport(CloseHandle(this->file));
			}
			else
			{
				TryAndReport(FreeResource(this->res_data));
			}

			this->res_data = NULL;
			this->file = INVALID_HANDLE_VALUE;
			WriteLog("datafile::open returning after a job well done");
		}

	private:
		// for files:
		HANDLE file;

		// for resources
		HGLOBAL res_data;
		BYTE *res_ptr; // pointer to our data
		BYTE *res_end; // end of resource data + 1
	};

	void datafile::open(const TCHAR *res_name,const TCHAR *res_type,HMODULE res_mod,WORD res_lang)
	{
		WriteLog("datafile::open (as resource) called for data file at memory address %8x" LogArg((DWORD)this));
		// try to Find, Lock, and Load the resource
		HRSRC res_handle = TryAndReport(FindResourceEx(res_mod,(LPCTSTR)res_type,(LPCTSTR)res_name,res_lang));
		if(NULL == res_handle)
		{
			this->file = INVALID_HANDLE_VALUE; // so we know we are invalid
			WriteLog("datafile::open failed! returing!");
			return;
		}
		this->res_data = TryAndReport(LoadResource(res_mod,res_handle));

		if(NULL == this->res_data)
		{
			this->file = INVALID_HANDLE_VALUE;
			WriteLog("datafile::open failed! returing!");
			return;
		}
		DWORD len = TryAndReport(SizeofResource(res_mod,res_handle));
		assert(0 != len);
		this->res_ptr = (BYTE *)TryAndReport(LockResource(res_data));
		this->res_end = this->res_ptr + len;
		if(NULL == this->res_ptr || this->res_ptr == this->res_end)
		{
			this->res_data = NULL;
			this->file = INVALID_HANDLE_VALUE;
			WriteLog("datafile::open failed! returing!");
			return;
		}
		WriteLog("datafile::open succeeded; returing");
	}


	static inline DWORD GetDWORD(datafile& s,bool as_rgb = false)
	{
		DWORD ret;
		s.get(ret,as_rgb);
		return ret;
	}

	static void TranslateColorNP(DWORD *x)
	{
		CColorNP trans(
			(BYTE)((*x) & 0xff),
			(BYTE)(((*x)>>8)&0xff),
			(BYTE)(((*x)>>16)&0xff)
		);
		trans.Certify();

		*x = trans.Color32b();
	}

	static void TranslateColor256(DWORD *x)
	{
		CColor256 trans(
			(BYTE)((*x) & 0xff),
			(BYTE)(((*x)>>8)&0xff),
			(BYTE)(((*x)>>16)&0xff)
		);
		trans.Certify();

		*x = trans.Color();
	}

	int CCompactMap::Certify()
	{
		WriteLog("CCompactMap::Certify called for CCompact map at memory address %8x" LogArg((DWORD)this));
		// sanity checks
		if(this->Certified())
		{
			WriteLog("CCompactMap::Certify returning because we are already certified");
			return 0;
		}

		bool useCColor256 = bool(1 == CGraphics::BytesPerPixel());

		WriteLog("bool useCColor256 = %x" LogArg((DWORD)useCColor256));

		datafile f;

		WriteLog("About to open our file from...");
		if(true == this->load_from_resource)
		{
			WriteLog("A resource...");
			f.open(this->file_name.c_str(),this->resource_type.c_str(),this->resource_module,this->resource_language);
		}
		else
		{
			WriteLog("A data file...");
			f.open(this->file_name.c_str());
		}

		typedef void (*TRANSLATECOLOR)(DWORD *);
		TRANSLATECOLOR tc = (true == useCColor256) ? TranslateColor256 : TranslateColorNP;

		if(f.fail())
		{
			WriteLog("We failed to open the CCompactMap data, so we are now returning");
			return 2;
		}

		// get number of block colors
		this->blocks.resize(GetDWORD(f));
		this->block_areas.resize(this->blocks.size());
		WriteLog("Number of block colors is %d" LogArg(this->blocks.size()));

		VCTR_DWORD::iterator block_i;
		VCTR_RECTANGLEVECTOR::iterator area_i;
		for(block_i = this->blocks.begin(),area_i = this->block_areas.begin();block_i != this->blocks.end();block_i++,area_i++)
		{
			*block_i = GetDWORD(f,true); // translate its color into a usable form
			tc(block_i);                 // 
			area_i->resize(GetDWORD(f)); // get how many blocks of it we have

			WriteLog("Getting data for a another color block of color %x and the number of blocks of this color being %d" LogArg(*block_i) LogArg(area_i->size()));

			for
			(
				VCTR_RECTANGLE::iterator rect_i = area_i->begin();
				rect_i != area_i->end();
				rect_i++
			)
			{
				rect_i->bottom = GetDWORD(f);
				rect_i->left = GetDWORD(f);
				rect_i->right = GetDWORD(f);
				rect_i->top = GetDWORD(f);
				WriteLog("This block has rect coordinates of %dx%d-%dx%d" LogArg(rect_i->left) LogArg(rect_i->top) LogArg(rect_i->right) LogArg(rect_i->bottom));
			}
		}

		WriteLog("Finished with the color block data.  Now onto patterns...");

		this->patterns.resize(GetDWORD(f)); 		// get number of patterns
		this->pattern_coors.resize(this->patterns.size());
		WriteLog("We have %i patterns" LogArg(this->patterns.size()));
		
		VCTR_DIRECTDRAWSURFACE::iterator pattern_i;
		for(pattern_i = this->patterns.begin(),area_i = this->pattern_coors.begin();pattern_i != this->patterns.end();	pattern_i++,area_i++)
		{
			WriteLog("Getting data for another pattern");
			DWORD width = (DWORD)GetDWORD(f);
			DWORD height = (DWORD)GetDWORD(f);

			WriteLog("Creating a surface for this pattern");
			LPDIRECTDRAW2 dd = CGraphics::DirectDraw();
			
			// setup a surface description
			DDSURFACEDESC sd;
			memset((void *)&sd,0,sizeof(sd));

			sd.dwSize = sizeof(sd);
			sd.dwWidth = width;
			sd.dwHeight = height;
			sd.ddsCaps.dwCaps |= DDSCAPS_OFFSCREENPLAIN;
			sd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY; // default to using video memory
			sd.dwFlags |= DDSD_CAPS;
			sd.dwFlags |= DDSD_HEIGHT;
			sd.dwFlags |= DDSD_WIDTH;

			LPDIRECTDRAWSURFACE version1_of_pattern_i;

			if(FAILED(TryAndReport(dd->CreateSurface(&sd,&version1_of_pattern_i,NULL))))
			{
				WriteLog("We failed to make the surface in video memory, now trying for system memory");
				sd.ddsCaps.dwCaps &=
					~DDSCAPS_VIDEOMEMORY; 
				sd.ddsCaps.dwCaps |=
					DDSCAPS_SYSTEMMEMORY;
				if(FAILED(TryAndReport(dd->CreateSurface(&sd,&version1_of_pattern_i,NULL))))
				{
					WriteLog("Failed to create the surface in system memory also!");
					this->patterns.resize(pattern_i-this->patterns.begin());
					this->pattern_coors.resize(area_i-this->pattern_coors.begin());
					CCertifiable::Certify();
					WriteLog("Returning so the user can at least see what we have managed to load so far (no patterns, only color blocks)");
					return 1;
				}
			}

			WriteLog("Getting the newer version of DirectDraw surface by using QueryInterface");
			HRESULT res;
			MemoryAllocFunction(
				res = TryAndReport(version1_of_pattern_i->QueryInterface(IID_IDirectDrawSurface2,(void **)pattern_i)),
				sizeof(IDirectDrawSurface2),
				FAILED(res)
			);

			WriteLog("Releasing old version of the DirectDraw surface");
			TryAndReport(version1_of_pattern_i->Release());

			// clear out the surface description again again
			memset((void *)&sd,0,sizeof(sd));
			sd.dwSize = sizeof(sd);

			WriteLog("Locking the surface so we can load the pixel data into it"); 
			TryAndReport((**pattern_i).Lock(NULL,&sd,DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT | DDLOCK_WRITEONLY,NULL));

			BYTE *buffer = (BYTE *)sd.lpSurface;

			// calculate bytes per pixel (since we have the surface locked and everything . . . )
			if(NULL == buffer) {TryAndReport((**pattern_i).GetSurfaceDesc(&sd));}

			int bypp = sd.ddpfPixelFormat.dwRGBBitCount;
			bypp += bypp%8;
			bypp /= 8;

			WriteLog("Using %d bytes per pixel" LogArg(bypp));


			bool surface_locked_successfully;
			if(NULL == buffer)
			{
				WriteLog("We failed in locking the surface, gonna leave this surface unchanged and hope it doesn't have a huge visual impact");
				surface_locked_successfully = false;
				WriteLog("Creating a dummy array to hold the buffer");
				buffer = (BYTE *)TryAndReport(new BYTE[width*bypp]);
				// supply a dummy pitch that matches the dummy buffer
				sd.lPitch = 0;
			}
			else
			{
				WriteLog("We know the surface was locked successfully");
				surface_locked_successfully = true;
			}

			if(false == useCColor256)
			{
				WriteLog("Loading pixels without using 256 colors");
				DWORD *to_write;

				for(DWORD y = 0; y < height; y++)
				{
					for(DWORD x = 0; x < width; x++)
					{
						to_write = (DWORD *)&buffer[x*bypp];

						*to_write = GetDWORD(f,true);

						tc(to_write);
					}
					buffer += sd.lPitch;
				}
			}
			else
			{
				WriteLog("Loading pixels using 256 colors");
				for(DWORD y = 0; y < height; y++)
				{
					for(DWORD x = 0; x < width; x++)
					{
						DWORD next = GetDWORD(f,true);

						tc(&next);
						
						buffer[x] = (BYTE)next;
					}
					buffer += sd.lPitch;
				}
			}

			WriteLog("Finished loading pixels");

			if(true == surface_locked_successfully)
			{
				WriteLog("Unlocking surface");
				TryAndReport((**pattern_i).Unlock(sd.lpSurface));
			}
			else
			{
				WriteLog("Deleting dummy buffer");
				delete buffer;
			}

			WriteLog("Getting the rectangles which tell us where the patterns are drawn");
			area_i->resize(GetDWORD(f));
			WriteLog("There are %d rectangles for this pattern" LogArg(area_i->size()));

			for(VCTR_RECTANGLE::iterator rect_i = area_i->begin(); rect_i != area_i->end(); rect_i++)
			{
				rect_i->bottom = GetDWORD(f);
				rect_i->left = GetDWORD(f);
				rect_i->right = GetDWORD(f);
				rect_i->top = GetDWORD(f);
				WriteLog("This block has rect coordinates of %dx%d-%dx%d" LogArg(rect_i->left) LogArg(rect_i->top) LogArg(rect_i->right) LogArg(rect_i->bottom));
			}
		}

		WriteLog("All finished loading pattern data, now onto getting \"leftover\" data");

		// find pixel size in bytes
		int pixel_size = CGraphics::BytesPerPixel();
		// get number of rows
		this->left_over.resize(GetDWORD(f));
		// prepare min/max finding
		this->min_max_x_row_offsets.resize(this->left_over.size());

		VCTR_MINMAXXROWOFFSET::iterator minmax_i;
		for
		(
			minmax_i = this->min_max_x_row_offsets.begin();
			minmax_i != this->min_max_x_row_offsets.end();
			minmax_i++
		)
		{
			minmax_i->first = 2147483647L;
			minmax_i->second = -(long)2147483648L;
		}
		this->min_max_x_offsets.first = 2147483647L;
		this->min_max_x_offsets.second = -(long)2147483648L;

		VCTR_ROW::iterator row_i;
		for
		(
			row_i = this->left_over.begin(), minmax_i = this->min_max_x_row_offsets.begin();
			row_i != this->left_over.end() /*&& minmax_i != this->"".end()*/;
			row_i++,
			minmax_i++
		)
		{
			row_i->resize(GetDWORD(f));
			// now get each left over run in this row
			int current_offset = 0;
			VCTR_RUN::iterator run_i;
			for(run_i = row_i->begin(); run_i != row_i->end(); run_i++)
			{
				// first get number of transparent pixels in this run
				current_offset += GetDWORD(f) * pixel_size;
				// add it to the offset of this non-transparent run
				run_i->first = current_offset;
				
				// see if it beats the min record
				if(minmax_i->first > current_offset)
				{
					minmax_i->first = current_offset;
				}
				if(this->min_max_x_offsets.first > current_offset)
				{
					this->min_max_x_offsets.first = current_offset;
				}
								
				// make room for the next non-transparent run
				run_i->second.resize(GetDWORD(f)*pixel_size);
				VCTR_BYTE::iterator byte_i;
				for(byte_i = run_i->second.begin(); byte_i != run_i->second.end(); byte_i+=pixel_size)
				{
					// get the color in this array
					DWORD color;
					color = GetDWORD(f,true);
					tc(&color);
					memcpy((void *)byte_i,(const void *)&color,pixel_size);
				}
				current_offset += run_i->second.size() + 1;

				// see if it beats the max record
				if(minmax_i->second < current_offset)
				{
					minmax_i->second = current_offset;
				}
				if(this->min_max_x_offsets.second < current_offset)
				{
					this->min_max_x_offsets.second = current_offset;
				}
			}
		}

		WriteLog("Finished getting leftovers, now closing CCompactMap data file");
		f.close();

		WriteLog("CCompactMap::Certify is returning after a successful job well-done");
		return CCertifiable::Certify();
	}

} // end of namespace


