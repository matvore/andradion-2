#include "stdafx.h"
#include "Certifiable.h"
#include "SurfaceLock.h"
#include "SurfaceLock256.h"

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

  int x = 1;
  
  asm volatile
    (
      "cmpl $0,%[xdif];"
      "jge no_reverse_x_stuff;"
      "negl %[xdif];"
      "negl %[xinc];"
      "no_reverse_x_stuff:"
      "cmpl $0,%[ydif];"
      "jge no_reverse_y_stuff;"
      "negl %[ydif];"
      "negl %[yinc];"
      "no_reverse_y_stuff:"
      "cmpl %[ydif],%[xdif];"
      "jg draw_small_slope;"
      "movl %[xdif],%%ecx;" 
      "movl %[ydif],%[xdif];" 
      "movl %%ecx,%[ydif];" 
      "movl %[xinc],%%ecx;" 
      "movl %[yinc],%[error];"
      "movl %[error],%[xinc];"
      "movl %%ecx,%[yinc];" 
      "draw_small_slope:"
      "cmpl $0,%[xdif];"
      "je lets_quit;"
      "movl $0,%[error];"
      // start a loop
      "movl %[xdif],%%ecx;"
      "loop_small_slope:"
      "testl $1,%[pattern];"
      "jz no_set_color_1;"
      "movb %[color],(%[addr]);"
      "no_set_color_1:"
      "rorl $1,%[pattern];"
      "addl %[ydif],%[error];"
      "cmpl %[xdif],%[error];"
      "jle no_handle_overflow_1;"
      "subl %[xdif],%[error];"
      "addl %[yinc],%[addr];"
      "no_handle_overflow_1:"
      "addl %[xinc],%[addr];"
      "loop loop_small_slope;"
      "lets_quit:"
      :
      [xinc] "+X" (x), 
      [yinc] "+X" (ddsd.lPitch),
      [ydif] "+X" (y1 - y0)
      :
      [error] "r" (0),
      [color] "q" (color),
      [xdif] "r" (x1 - x0),
      [pattern] "X" (pattern),
      [addr] "r" ((BYTE *)this->ddsd.lpSurface + (y0 * ddsd.lPitch) + x0)
      : "ecx"
      );

}


