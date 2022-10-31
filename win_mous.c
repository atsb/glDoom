// win_mous.c
// This is the mouse support module for Win32
//
#include <windows.h>
#include <dinput.h>

#include "sys_win.h"
#include "d_main.h"
#include "d_event.h"
#include "dxerr.h"

extern  LPDIRECTINPUT lpDirectInput;
LPDIRECTINPUTDEVICE   lpMouse       = 0;

extern int        mouseb1, mouseb2, mouseb3;
extern int        mvert;
extern windata_t  WinData;
extern POINT      mPoint;

void lfprintf(char *message, ... );

void I_ReleaseMouse()
   {
    if (lpMouse != 0)
       {
        lpMouse->lpVtbl->Unacquire(lpMouse);
        lpMouse->lpVtbl->Release(lpMouse);
        lpMouse = 0;
       }
   }

dboolean I_SetupMouse()
   {
    HRESULT          hresult;
    
    hresult = lpDirectInput->lpVtbl->CreateDevice(lpDirectInput, &GUID_SysMouse, &lpMouse, NULL );
    if (hresult != S_OK)
       {
        DI_Error( hresult, "CreateDevice (mouse)");
        return false;
       }

    hresult = lpMouse->lpVtbl->SetDataFormat(lpMouse, &c_dfDIMouse);
    if (hresult != S_OK)
       {
        DI_Error( hresult, "SetDataFormat (mouse)");
        I_ReleaseMouse();
        return false;
       }

    hresult = lpMouse->lpVtbl->SetCooperativeLevel(lpMouse, WinData.hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
    if (hresult != S_OK)
       {
        DI_Error( hresult, "SetCooperativeLevel (mouse)");
        I_ReleaseMouse();
        return false;
       }

    hresult = lpMouse->lpVtbl->Acquire(lpMouse);
    if (hresult != S_OK)
       {
        DI_Error( hresult, "Acquire (mouse)");
        I_ReleaseMouse();
        return false;
       }
    return true;
   }

void I_CheckMouse()
   {
    DIMOUSESTATE        diMouseState;          /* DirectInput mouse state structure */
    static DIMOUSESTATE diHoldMouse;           /* DirectInput mouse state structure */
    HRESULT             hresult;
    static event_t      mev[5];

    if (lpMouse != 0)
       {
        RetryMouse:;

        hresult = lpMouse->lpVtbl->GetDeviceState(lpMouse, sizeof(DIMOUSESTATE), &diMouseState);
        if ((hresult == MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, ERROR_READ_FAULT)) || (hresult == MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, ERROR_INVALID_ACCESS)))
           {
            hresult = lpMouse->lpVtbl->Acquire(lpMouse);
            if (SUCCEEDED(hresult))
                goto RetryMouse;
           }
        else
        if (hresult != S_OK)
           {
            DI_Error(hresult, "GetDeviceState(mouse)");
           }

        if (SUCCEEDED(hresult))
           {
            if ((diMouseState.lX != diHoldMouse.lX) ||
                (diMouseState.lY != diHoldMouse.lY) ||
                (diMouseState.lZ != diHoldMouse.lZ) ||
                (diMouseState.rgbButtons[0] != diHoldMouse.rgbButtons[0]) ||
                (diMouseState.rgbButtons[1] != diHoldMouse.rgbButtons[1]) ||
                (diMouseState.rgbButtons[2] != diHoldMouse.rgbButtons[2]) ||
                (diMouseState.rgbButtons[3] != diHoldMouse.rgbButtons[3]))
               {
                mev[4].type = ev_mouse;
                mev[4].data1 = 0;
                if (mouseb1 == 0)
                    mev[4].data1 |= ((diMouseState.rgbButtons[0] & 0x80) >> 7);
                else
                   {
                    if ((diHoldMouse.rgbButtons[0] & 0x80) != (diMouseState.rgbButtons[0] & 0x80))
                       {
                        if ((diMouseState.rgbButtons[0] & 0x80) == 0x80)
                           {
                            mev[0].type = ev_keydown;
                           }
                        else
                           {
                            mev[0].type = ev_keyup;
                           }
                        mev[0].data1 = mouseb1;
                        D_PostEvent(&mev[0]);
                       }
                   }
                if (mouseb2 == 0)
                    mev[4].data1 |= ((diMouseState.rgbButtons[1] & 0x80) >> 6);
                else
                   {
                    if ((diHoldMouse.rgbButtons[1] & 0x80) != (diMouseState.rgbButtons[1] & 0x80))
                       {
                        if ((diMouseState.rgbButtons[1] & 0x80) == 0x80)
                           {
                            mev[1].type = ev_keydown;
                           }
                        else
                           {
                            mev[1].type = ev_keyup;
                           }
                        mev[1].data1 = mouseb2;
                        D_PostEvent(&mev[1]);
                       }
                   }
                if (mouseb3 == 0)
                   {
                    mev[4].data1 |= ((diMouseState.rgbButtons[2] & 0x80) >> 5);
                   }
                else
                   {
                    if ((diHoldMouse.rgbButtons[2] & 0x80) != (diMouseState.rgbButtons[2] & 0x80))
                       {
                        if ((diMouseState.rgbButtons[2] & 0x80) == 0x80)
                           {
                            mev[2].type = ev_keydown;
                           }
                        else
                           {
                            mev[2].type = ev_keyup;
                           }
                        mev[2].data1 = mouseb3;
                        D_PostEvent(&mev[2]);
                       }
                   }
                mev[4].data2 = diMouseState.lX*3;
                if (mvert == true)
                    mev[4].data3 = (diMouseState.lY * -3);
                else
                    mev[4].data3 = 0;
                D_PostEvent(&mev[4]);
                diHoldMouse.lX = diMouseState.lX;
                diHoldMouse.lY = diMouseState.lY;
                diHoldMouse.lZ = diMouseState.lZ;
                diHoldMouse.rgbButtons[0] = diMouseState.rgbButtons[0];
                diHoldMouse.rgbButtons[1] = diMouseState.rgbButtons[1];
                diHoldMouse.rgbButtons[2] = diMouseState.rgbButtons[2];
                diHoldMouse.rgbButtons[3] = diMouseState.rgbButtons[3];
                SetCursorPos(mPoint.x, mPoint.y);
               }
           }
       }
   }
