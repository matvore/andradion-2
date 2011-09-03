#ifndef __GAMELIB_H__
#define __GAMELIB_H__

// the main basic PimeLib header
#include <windows.h>
#include <cstring>
#include <cstdlib>
#include <new.h>

// safe deletes and releases
#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

// Convert from multibyte format to Unicode using the following macro:
#define MULTI_TO_WIDE( x,y )  MultiByteToWideChar( CP_ACP,MB_PRECOMPOSED, y,-1,x,DMUS_MAX_FILENAME);

// playing of SOB's:
#define Play_SOB(s,l) (s)->Play(0,0,(l) ? DSBPLAY_LOOPING : 0)
// s: the SOB to play
// l: bool, true: loop over and over until stopped with other function
//          false: play once

// here we'll silence a SOB if its playing
#define Silence_SOB(s) (s)->Stop()

// this one is just for zeroingmemory of a struct and setting the dwSize member
#define DX_Init_Struct(s) {memset(&(s),0,sizeof(s)); (s).dwSize = sizeof(s);}

// here is a set of macros that make it easier to put to put together game loops in
//  this windows model, whereas, you must return -1 every frame and then be able to
//  pick up where you left off

#define Frame_Return_Able int // use as the return type of the function

// the negative values that can be returned are only utilized by the
//  next macros, but the final return value can be defined in Leave()
//  macro.  Never Return for repeat on the first return index (0)

// next macro put in the variable definitions to enable frame return
#define Enable_Frame_Return() static int __Frame_Return_Index = 0

// next one put a bunch of, one for each point, right after var decs
//  give it a unique index >= 0 and name
#define Init_Return_Point(index,name) if(__Frame_Return_Index == (index)) goto name

// next macro, put wherever there is a return point
#define Define_Return_Point(name) name:

// next put it whenever you want to return and reset the frame return index
#define Leave(return_value) {__Frame_Return_Index = 0; return return_value;}

// next one returns without incrementing the return index
#define Stop_For_Repeat() return (-__Frame_Return_Index)

// next one returns with incrementing the return index
#define Stop_For_Progress() return (-(++__Frame_Return_Index))

// here are some more macros for syncronization: //

// syncing functions:
#define Enable_Sync() DWORD __sync_system_start

// next macro starts timer
#define Start_Frame_For_Sync() __sync_system_start = GetTickCount()

// next macro to be put at the end of the game loop:
#define End_Frame_For_Sync(fps) while(float(GetTickCount()-__sync_system_start) * (float)(fps) < 1000.0) 
#define AND_WAIT_FOR_VR { ddraw_int->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN,0); }

// here we have the super-cool error-handling macros:

// use this to define any function using this system
typedef int Error_Ready;

// use this to initiate the system in any function that uses it
#define Init_Error_Ready_System() int __ERS_error_value; char __ERS_error_message[256] = "\0"


#define Return_With_Error(error_code) {__ERS_error_value = error_code; goto __ERSLABEL__;}

#define Begin_Error_Table \
	return 0; \
__ERSLABEL__: \
	switch(__ERS_error_value) {

#define Begin_Error_Table_Return_Other_On_Success(x) \
	return x; \
__ERSLABEL__: \
	switch(__ERS_error_value) {

#define Begin_Error_Table_Return_Nothing \
	return;   \
__ERSLABEL__: \
	switch(__ERS_error_value) {

const char ERS_ERROR_MSG_CAPTION[] = "ERS Error";

#define Define_Error(x,msg) case x: if(__ERS_error_message[0] == 0) strcpy(__ERS_error_message,msg);

#define End_Error_Table(window_handle) \
	default: break; }; MessageBox(window_handle,__ERS_error_message,ERS_ERROR_MSG_CAPTION,MB_ICONSTOP); return __ERS_error_value
#define End_Error_Table_Return_Other_On_Failure(window_handle,x) \
	default: break; }; MessageBox(window_handle,__ERS_error_message,ERS_ERROR_MSG_CAPTION,MB_ICONSTOP); return (x)
#define End_Error_Table_Return_Nothing(window_handle) \
	default: break; }; MessageBox(window_handle,__ERS_error_message,ERS_ERROR_MSG_CAPTION,MB_ICONSTOP); return
#define End_Error_Table_Exit_With_Error(window_handle) \
	default: break; }; MessageBox(window_handle,__ERS_error_message,ERS_ERROR_MSG_CAPTION,MB_ICONSTOP); exit(1)

#define Memory_Alloc_Function(op,size,condition) \
do {int newres; _PNH newhandler; op; \
if(condition) { \
newhandler = _set_new_handler(NULL); \
_set_new_handler(newhandler); \
newres = newhandler(size); \
_set_new_handler(newhandler); \
if(0 == newres)	{MessageBox(NULL,"Out of memory.",NULL,MB_ICONSTOP);exit(1);}} \
} while(condition)

#endif
