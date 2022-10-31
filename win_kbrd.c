// win_kbrd.c
//
// This module handles the DirectInput startup code and initializes
// the keyboard interface as well as handling the keyboard input
// and shutting down the keyboard interface and DirectInput.

#include <windows.h>
#include <dinput.h>
#include <stdio.h>

#include "doomtype.h"
#include "d_main.h"
#include "d_event.h"
#include "dxerr.h"
#include "sys_win.h"

#undef RELEASE
#ifdef __cplusplus
#define RELEASE(x) if (x != NULL) {x->Release(); x = NULL;}
#else
#define RELEASE(x) if (x != NULL) {x->lpVtbl->Release(x); x = NULL;}
#endif

LPDIRECTINPUT        lpDirectInput = 0;
LPDIRECTINPUTDEVICE  lpKeyboard    = 0;

unsigned char        KeyState[256];
short                si_Kbd[256];

extern int           keylink;

extern windata_t     WinData;

void lfprintf(char *message, ... );

void I_ReleaseKeyboard()
   {
    if (lpKeyboard != 0)
       {
        lpKeyboard->lpVtbl->Unacquire(lpKeyboard);
        lpKeyboard->lpVtbl->Release(lpKeyboard);
        lpKeyboard = 0;
       }
    RELEASE(lpDirectInput);
   }

dboolean I_SetupKeyboard()
   {
    int     k;
    HRESULT hresult;

    hresult = DirectInput8Create(WinData.hInstance, DIRECTINPUT_VERSION, &IID_IDirectInput8, &lpDirectInput, NULL );
    if (hresult != S_OK)
       {
        DI_Error( hresult, "DirectInputCreate");
        return false;
       }
    hresult = lpDirectInput->lpVtbl->CreateDevice(lpDirectInput, &GUID_SysKeyboard, &lpKeyboard, NULL );
    if (hresult != S_OK)
       {
        DI_Error( hresult, "CreateDevice (keyboard)");
        I_ReleaseKeyboard();
        return false;
       }
    hresult = lpKeyboard->lpVtbl->SetDataFormat(lpKeyboard, &c_dfDIKeyboard);
    if (hresult != S_OK)
       {
        DI_Error( hresult, "SetDataFormat (keyboard)");
        I_ReleaseKeyboard();
        return false;
       }
    hresult = lpKeyboard->lpVtbl->SetCooperativeLevel(lpKeyboard, WinData.hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
    if (hresult != S_OK)
       {
        DI_Error( hresult, "SetCooperativeLevel (keyboard)");
        I_ReleaseKeyboard();
        return false;
       }
    hresult = lpKeyboard->lpVtbl->Acquire(lpKeyboard);
    if (hresult != S_OK)
       {
        DI_Error( hresult, "Acquire (keyboard)");
        I_ReleaseKeyboard();
        return false;
       }

    // Set the keyboard buffer to "all keys up"
    for (k = 0; k < 256; k++)
        si_Kbd[k] = WM_KEYUP;

    return true;
   }

char t_text[2048];

void I_CheckKeyboard()
   {
    HRESULT          hresult;
    static  event_t  event[256];
    unsigned short   lm;
    int     i;

    if (lpKeyboard == 0)
       {
        return;
       }

    RetryKeyboard:;

    hresult = lpKeyboard->lpVtbl->GetDeviceState(lpKeyboard, sizeof(KeyState), &KeyState);
    if ((hresult == MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, ERROR_READ_FAULT)) || (hresult == MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, ERROR_INVALID_ACCESS)))
       {
        hresult = lpKeyboard->lpVtbl->Acquire(lpKeyboard);
        if (SUCCEEDED(hresult))
            goto RetryKeyboard;
       }
    else
    if (hresult != S_OK)
       {
        DI_Error(hresult, "GetDeviceState (keyboard)");
        sprintf(t_text, "hresult = %08X\n", hresult);
        lm = 0;
       }
    else
       {
        for (i = 1; i < 256; i++)
           {
                if (((KeyState[i] & 0x80) == 0) && (si_Kbd[i] == WM_KEYDOWN))
                   {
                    event[i].type = ev_keyup;
                    event[i].data1 = i;
                    D_PostEvent(&event[i]);
                    si_Kbd[i] = WM_KEYUP;
                   }

                if ((KeyState[i] & 0x80) && (si_Kbd[i] == WM_KEYUP))
                   {
                    if ((i != WS_TABSTOP) || ((KeyState[VK_LMENU] == WM_KEYUP) && (KeyState[VK_RMENU] == WM_KEYUP)))
                       {
                        event[i].type = ev_keydown;
                        event[i].data1 = i;
                        D_PostEvent(&event[i]);
                        si_Kbd[i] = WM_KEYDOWN;
                       }
                   }
           }
       }
   }
