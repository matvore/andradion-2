#include "Certifiable.h"
#include "SurfaceLock.h"
#include "SurfaceLock256.h"

// Comment the next three lines to allow this module to log
#define WriteLog(x)
#undef LogArg
#define LogArg(x)
#undef TryAndReport
#define TryAndReport(x) x

namespace NGameLib2
{
	CSurfaceLock256::CSurfaceLock256() : CSurfaceLock()	{}

	CSurfaceLock256::CSurfaceLock256
		(
		LPDIRECTDRAWSURFACE2 target_surface_,
		RECT *area_,DWORD lock_flags_
		) : CSurfaceLock(target_surface_,area_,lock_flags_)
	{
	}

	void CSurfaceLock256::Line(int x0,int y0,int x1,int y1,DWORD color,DWORD pattern)
	{
		assert(this->Certified());
		// okay, now call the target surface's GetDC function . . . .
		//  just kidding!

		// make shortcut variables for parts of the surface struct
		unsigned long p = this->ddsd.lPitch;
		BYTE *s = (BYTE *)this->ddsd.lpSurface + (y0 * p) + x0;

		DWORD pattern2 = pattern; // keep a backup copy of the pattern parameter
		int count_down_to_repeat = 32;

		// analyze delta values
		int x_inc;
		int y_inc;

		int error = 0;

		int ydiff;

		asm
		{
			mov edx,y1
			sub edx,y0
			mov ydiff,edx
			mov edx,x1
			sub edx,x0

			mov ecx,1
			cmp edx,0			
			jge no_reverse_x_stuff
			neg edx
			neg ecx
no_reverse_x_stuff:
			mov x_inc,ecx
			mov ecx,p
			cmp ydiff,0			
			jge no_reverse_y_stuff
			neg ydiff
			neg ecx
no_reverse_y_stuff:
			mov y_inc,ecx
			cmp edx,ydiff
			jg draw_small_slope
			// if we are here, then the slope is big and we should swap edx and ydiff
			mov ecx,edx
			mov edx,ydiff
			mov ydiff,ecx
			// swap x_inc and y_inc
			mov ecx,x_inc
			mov ebx,y_inc
			mov x_inc,ebx
			mov y_inc,ecx

draw_small_slope:
			cmp edx,0
			je lets_quit
			// start a loop
			mov ecx,edx 

loop_small_slope:
			mov eax,pattern
			and eax,1
			cmp eax,1
			jne no_set_color_1
			mov eax,s
			mov ebx,color
			mov BYTE PTR [eax],bl
no_set_color_1:
			shr pattern,1
			dec count_down_to_repeat
			cmp count_down_to_repeat,0
			jg no_reset_pattern_1
			mov count_down_to_repeat,32
			mov eax,pattern2
			mov pattern,eax
no_reset_pattern_1:
			mov eax,error
			add eax,ydiff
			mov error,eax
			mov eax,s
			cmp error,edx
			jle no_handle_overflow_1
			sub error,edx
			add eax,y_inc
no_handle_overflow_1:
			add eax,x_inc
			mov s,eax
			loop loop_small_slope
			// all done with rendering line
lets_quit:
		} // end line-drawing assembly
	}
}


