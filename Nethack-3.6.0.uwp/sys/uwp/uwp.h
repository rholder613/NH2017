/* NetHack 3.6	uwp.h	$NHDT-Date:  $  $NHDT-Branch:  $:$NHDT-Revision:  $ */
/* Copyright (c) Bart House, 2016-2017. */
/* Nethack for the Universal Windows Platform (UWP) */
/* NetHack may be freely redistributed.  See license for details. */
#pragma once

#define _USE_MATH_DEFINES

#include <windows.h>
#include <assert.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector> 
#include <streambuf>
#include <sstream>
#include <setjmp.h>

#include <wrl.h>
#include <wrl/client.h>
#include <dxgi1_4.h>
#include <d3d11_3.h>
#include <d2d1_3.h>
#include <d2d1effects_2.h>
#include <dwrite_3.h>
#include <wincodec.h>
#include <DirectXColors.h>
#include <DirectXMath.h>
#include <memory>
#include <agile.h>
#include <concrt.h>

#include <wrl.h>
#include <wrl/client.h>
#include <dxgi1_4.h>
#include <d3d11_3.h>
#include <d2d1_3.h>
#include <d2d1effects_2.h>
#include <dwrite_3.h>
#include <wincodec.h>
#include <DirectXColors.h>
#include <DirectXMath.h>
#include <memory>
#include <agile.h>
#include <concrt.h>
#include <collection.h>
#include <ppltasks.h>

#include <wrl.h>
#include <wrl/client.h>
#include <dxgi1_4.h>
#include <d3d11_3.h>
#include <d2d1_3.h>
#include <d2d1effects_2.h>
#include <dwrite_3.h>
#include <wincodec.h>
#include <DirectXColors.h>
#include <DirectXMath.h>
#include <memory>

#include <agile.h>
#include <concrt.h>
#include <math.h>
#include <d3d11_2.h>
#include <d2d1_2.h>
#include <dwrite_2.h>
#include <wincodec.h>
#include <DirectXMath.h>
#include <agile.h>
#include <d3d11_3.h>
#include <dxgi1_4.h>
#include <d2d1_3.h>
#include <wrl.h>
#include <wrl/client.h>
#include <d2d1effects_2.h>
#include <dwrite_3.h>
#include <wincodec.h>
#include <DirectXColors.h>
#include <DirectXMath.h>
#include <memory>
#include <agile.h>
#include <concrt.h>

#include "uwpenum.h"
#include "uwpmath.h"
#include "uwplock.h"
#include "uwputil.h"
#include "uwpconditionvariable.h"
#include "uwpshaderstructures.h"
#include "uwpasciitexture.h"
#include "uwptextgrid.h"
#include "uwpoption.h"
#include "uwpfilehandler.h"
#include "uwpeventqueue.h"
#include "uwpdeviceResources.h"
#include "uwpsteptimer.h"
#include "uwpnethackmain.h"
#include "uwpapp.h"
#include "uwpfont.h"
#include "uwpdxhelper.h"
#include "uwpglobals.h"


extern"C" {

    #include "hack.h"

    #ifdef TTY_GRAPHICS
    #include "wintty.h"
    #endif

    #include "spell.h"

    #include "date.h"
    #include "patchlevel.h"
    #include "dlb.h"


    extern char MapScanCode(const Nethack::Event & e);
    extern int raw_getchar();

}
