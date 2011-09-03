#include "StdAfx.h"
#include "Fixed.h"

FIXEDNUM FixedDiv(FIXEDNUM x,FIXEDNUM y) {
  FIXEDNUM r;
  asm (
     "movl %1,%%eax;"
     "cdq;"
     "shldl $16,%%eax,%%edx;"
     "sall $16,%%eax;"
     "idivl %2;"
     "shldl $16,%%eax,%%edx;"
     "movl %%eax,%0;"
     : "=r" (r) : "r" (x) , "r" (y) : "eax", "edx");
  return r;
}

FIXEDNUM FixedMul(FIXEDNUM x,FIXEDNUM y) {
  FIXEDNUM r;
  asm (   
      "movl %1,%%eax;"
      "imull %2;"
      "shrdl $16,%%edx,%%eax;"
      "movl %%eax, %0;"
      : "=r" (r) : "r" (x) , "r" (y) : "eax", "edx");
  return r;
}
