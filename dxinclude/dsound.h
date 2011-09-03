/*==========================================================================;
 *
 *  Copyright (C) 1995,1996 Microsoft Corporation.  All Rights Reserved.
 *
 *  File:       dsound.h
 *  Content:    DirectSound include file
 *
 ***************************************************************************/

#define COM_NO_WINDOWS_H
#include <objbase.h>

#define _FACDS  0x878
#define MAKE_DSHRESULT( code )  MAKE_HRESULT( 1, _FACDS, code )

extern "C" {

DEFINE_GUID(CLSID_DirectSound,
0x47d4d946, 0x62e8, 0x11cf, 0x93, 0xbc, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0);
DEFINE_GUID(IID_IDirectSound,0x279AFA83,0x4981,0x11CE,0xA5,0x21,0x00,0x20,0xAF,0x0B,0xE5,0x60);
DEFINE_GUID(IID_IDirectSoundBuffer,0x279AFA85,0x4981,0x11CE,0xA5,0x21,0x00,0x20,0xAF,0x0B,0xE5,0x60);

/* 'struct' not 'class' per the way DECLARE_INTERFACE_ is defined */
struct IDirectSound;
struct IDirectSoundBuffer;
struct IDirectSound3DListener;
struct IDirectSound3DBuffer;

typedef struct IDirectSound           *LPDIRECTSOUND;        
typedef struct IDirectSoundBuffer     *LPDIRECTSOUNDBUFFER;  
typedef struct IDirectSoundBuffer    **LPLPDIRECTSOUNDBUFFER;  
typedef struct IDirectSound3DListener         *LPDIRECTSOUND3DLISTENER;
typedef struct IDirectSound3DBuffer   *LPDIRECTSOUND3DBUFFER;


typedef struct _DSCAPS {
  DWORD       dwSize;
  DWORD       dwFlags;
  DWORD       dwMinSecondarySampleRate;
  DWORD       dwMaxSecondarySampleRate;
  DWORD       dwPrimaryBuffers;
  DWORD       dwMaxHwMixingAllBuffers;
  DWORD       dwMaxHwMixingStaticBuffers;
  DWORD       dwMaxHwMixingStreamingBuffers;
  DWORD       dwFreeHwMixingAllBuffers;
  DWORD       dwFreeHwMixingStaticBuffers;
  DWORD       dwFreeHwMixingStreamingBuffers;
  DWORD       dwMaxHw3DAllBuffers;
  DWORD       dwMaxHw3DStaticBuffers;
  DWORD       dwMaxHw3DStreamingBuffers;
  DWORD       dwFreeHw3DAllBuffers;
  DWORD       dwFreeHw3DStaticBuffers;
  DWORD       dwFreeHw3DStreamingBuffers;
  DWORD       dwTotalHwMemBytes;
  DWORD       dwFreeHwMemBytes;
  DWORD       dwMaxContigFreeHwMemBytes;
  DWORD       dwUnlockTransferRateHwBuffers;
  DWORD       dwPlayCpuOverheadSwBuffers;
  DWORD       dwReserved1;
  DWORD       dwReserved2;
} DSCAPS, *LPDSCAPS;

typedef struct _DSBCAPS {
  DWORD       dwSize;
  DWORD       dwFlags;
  DWORD       dwBufferBytes;
  DWORD       dwUnlockTransferRate;
  DWORD       dwPlayCpuOverhead;
} DSBCAPS, *LPDSBCAPS;

typedef struct _DSBUFFERDESC {
  DWORD                   dwSize;
  DWORD                   dwFlags;
  DWORD                   dwBufferBytes;
  DWORD                   dwReserved;
  LPWAVEFORMATEX          lpwfxFormat;
} DSBUFFERDESC, *LPDSBUFFERDESC;

typedef LPVOID* LPLPVOID;

//
// IDirectSound
//
#undef INTERFACE
#define INTERFACE IDirectSound
DECLARE_INTERFACE_( IDirectSound, IUnknown ) {
  /*** IUnknown methods ***/
  STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
  STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
  STDMETHOD_(ULONG,Release) (THIS) PURE;
    
  /*** IDirectSound methods ***/
  STDMETHOD( CreateSoundBuffer)(THIS_ LPDSBUFFERDESC, LPLPDIRECTSOUNDBUFFER, IUnknown FAR *) PURE;
  STDMETHOD( GetCaps)(THIS_ LPDSCAPS ) PURE;
  STDMETHOD( DuplicateSoundBuffer)(THIS_ LPDIRECTSOUNDBUFFER, LPLPDIRECTSOUNDBUFFER ) PURE;
  STDMETHOD( SetCooperativeLevel)(THIS_ HWND, DWORD ) PURE;
  STDMETHOD( Compact)(THIS ) PURE;
  STDMETHOD( GetSpeakerConfig)(THIS_ LPDWORD ) PURE;
  STDMETHOD( SetSpeakerConfig)(THIS_ DWORD ) PURE;
  STDMETHOD( Initialize)(THIS_ const GUID * ) PURE;
};

//
// IDirectSoundBuffer
//
#undef INTERFACE
#define INTERFACE IDirectSoundBuffer
DECLARE_INTERFACE_( IDirectSoundBuffer, IUnknown ) {
  /*** IUnknown methods ***/
  STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
  STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
  STDMETHOD_(ULONG,Release) (THIS) PURE;

  /*** IDirectSoundBuffer methods ***/
  STDMETHOD(           GetCaps)(THIS_ LPDSBCAPS ) PURE;
  STDMETHOD(GetCurrentPosition)(THIS_ LPDWORD,LPDWORD ) PURE;
  STDMETHOD(         GetFormat)(THIS_ LPWAVEFORMATEX, DWORD, LPDWORD ) PURE;
  STDMETHOD(         GetVolume)(THIS_ LPLONG ) PURE;
  STDMETHOD(            GetPan)(THIS_ LPLONG ) PURE;
  STDMETHOD(      GetFrequency)(THIS_ LPDWORD ) PURE;
  STDMETHOD(         GetStatus)(THIS_ LPDWORD ) PURE;
  STDMETHOD(        Initialize)(THIS_ LPDIRECTSOUND, LPDSBUFFERDESC ) PURE;
  STDMETHOD(              Lock)(THIS_ DWORD,DWORD,LPVOID,LPDWORD,LPVOID,LPDWORD,DWORD ) PURE;
  STDMETHOD(              Play)(THIS_ DWORD,DWORD,DWORD ) PURE;
  STDMETHOD(SetCurrentPosition)(THIS_ DWORD ) PURE;
  STDMETHOD(         SetFormat)(THIS_ LPWAVEFORMATEX ) PURE;
  STDMETHOD(         SetVolume)(THIS_ LONG ) PURE;
  STDMETHOD(            SetPan)(THIS_ LONG ) PURE;
  STDMETHOD(      SetFrequency)(THIS_ DWORD ) PURE;
  STDMETHOD(              Stop)(THIS  ) PURE;
  STDMETHOD(            Unlock)(THIS_ LPVOID,DWORD,LPVOID,DWORD ) PURE;
  STDMETHOD(           Restore)(THIS  ) PURE;
};

#define DS_OK                           0
#define DSERR_ALLOCATED                 MAKE_DSHRESULT( 10 )
#define DSERR_CONTROLUNAVAIL            MAKE_DSHRESULT( 30 )
#define DSERR_INVALIDPARAM              E_INVALIDARG
#define DSERR_INVALIDCALL               MAKE_DSHRESULT( 50 )
#define DSERR_GENERIC                   E_FAIL
#define DSERR_PRIOLEVELNEEDED           MAKE_DSHRESULT( 70 )
#define DSERR_OUTOFMEMORY               E_OUTOFMEMORY
#define DSERR_BADFORMAT                 MAKE_DSHRESULT( 100 )
#define DSERR_UNSUPPORTED               E_NOTIMPL
#define DSERR_NODRIVER                  MAKE_DSHRESULT( 120 )
#define DSERR_ALREADYINITIALIZED        MAKE_DSHRESULT( 130 )
#define DSERR_NOAGGREGATION             CLASS_E_NOAGGREGATION
#define DSERR_BUFFERLOST                MAKE_DSHRESULT( 150 )
#define DSERR_OTHERAPPHASPRIO           MAKE_DSHRESULT( 160 )
#define DSERR_UNINITIALIZED             MAKE_DSHRESULT( 170 )

#define DSCAPS_PRIMARYMONO          0x00000001
#define DSCAPS_PRIMARYSTEREO        0x00000002
#define DSCAPS_PRIMARY8BIT          0x00000004
#define DSCAPS_PRIMARY16BIT         0x00000008
#define DSCAPS_CONTINUOUSRATE       0x00000010
#define DSCAPS_EMULDRIVER           0x00000020
#define DSCAPS_CERTIFIED            0x00000040
#define DSCAPS_SECONDARYMONO        0x00000100
#define DSCAPS_SECONDARYSTEREO      0x00000200
#define DSCAPS_SECONDARY8BIT        0x00000400
#define DSCAPS_SECONDARY16BIT       0x00000800

#define DSBPLAY_LOOPING                 0x00000001

#define DSBSTATUS_PLAYING           0x00000001
#define DSBSTATUS_BUFFERLOST        0x00000002
#define DSBSTATUS_LOOPING           0x00000004
         
#define DSBLOCK_FROMWRITECURSOR         0x00000001

#define DSSCL_NORMAL                1
#define DSSCL_PRIORITY              2
#define DSSCL_EXCLUSIVE             3
#define DSSCL_WRITEPRIMARY          4

#define DSBCAPS_PRIMARYBUFFER       0x00000001
#define DSBCAPS_STATIC              0x00000002
#define DSBCAPS_LOCHARDWARE         0x00000004
#define DSBCAPS_LOCSOFTWARE         0x00000008
#define DSBCAPS_CTRL3D              0x00000010
#define DSBCAPS_CTRLFREQUENCY       0x00000020
#define DSBCAPS_CTRLPAN             0x00000040
#define DSBCAPS_CTRLVOLUME          0x00000080
#define DSBCAPS_CTRLDEFAULT         0x000000E0  // Pan + volume + frequency.
#define DSBCAPS_CTRLALL             0x000000F0  // All control capabilities
#define DSBCAPS_STICKYFOCUS         0x00004000
#define DSBCAPS_GLOBALFOCUS         0x00008000 
#define DSBCAPS_GETCURRENTPOSITION2 0x00010000  // More accurate play cursor under emulation

#define DSSPEAKER_HEADPHONE     1
#define DSSPEAKER_MONO          2
#define DSSPEAKER_QUAD          3
#define DSSPEAKER_STEREO        4
#define DSSPEAKER_SURROUND      5

}
