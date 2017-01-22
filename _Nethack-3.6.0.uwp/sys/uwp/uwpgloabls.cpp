/* NetHack 3.6	uwpglobals.cpp	$NHDT-Date:  $  $NHDT-Branch:  $:$NHDT-Revision:  $ */
/* Copyright (c) Bart House, 2016-2017. */
/* Nethack for the Universal Windows Platform (UWP) */
/* NetHack may be freely redistributed.  See license for details. */
#include "uwp.h"

namespace Nethack
{

    const DirectX::XMFLOAT3 g_colorTable[] = {
        RGB_TO_XMFLOAT3(0x55, 0x55, 0x55), // black
        RGB_TO_XMFLOAT3(0xFF, 0x00, 0x00), // red
        RGB_TO_XMFLOAT3(0x00, 0x80, 0x00), // green
        RGB_TO_XMFLOAT3(205, 133, 63), // brown
        RGB_TO_XMFLOAT3(0x00, 0x00, 0xFF), // blue
        RGB_TO_XMFLOAT3(0xFF, 0x00, 0xFF), // magenta
        RGB_TO_XMFLOAT3(0x00, 0xFF, 0xFF), // cyan
        RGB_TO_XMFLOAT3(0x80, 0x80, 0x80), // gray
        RGB_TO_XMFLOAT3(0xFF, 0xFF, 0xFF), // bright
        RGB_TO_XMFLOAT3(0xFF, 0xA5, 0x00), // orange
        RGB_TO_XMFLOAT3(0x00, 0xFF, 0x00), // bright green
        RGB_TO_XMFLOAT3(0xFF, 0xFF, 0x00), // yellow
        RGB_TO_XMFLOAT3(0x00, 0xC0, 0xFF), // bright blue
        RGB_TO_XMFLOAT3(0xFF, 0x80, 0xFF), // bright magenta
        RGB_TO_XMFLOAT3(0x80, 0xFF, 0xFF), // bright cyan
        RGB_TO_XMFLOAT3(0xFF, 0xFF, 0xFF) // white
    };


    EventQueue      g_eventQueue;
    TextGrid        g_textGrid(Int2D(80, 25));
    jmp_buf         g_mainLoopJmpBuf;
    Options         g_options;
    FontCollection  g_fontCollection;
    std::string     g_installDir;
    std::string     g_localDir;
    std::string     g_defaultsFileName("defaults.nh");
    std::string     g_defaultsFilePath;
    std::string     g_nethackOptionsFileName("nethackoptions");
    std::string     g_nethackOptionsFilePath;
    std::string     g_guidebookFileName("Guidebook.txt");
    std::string     g_guidebookFilePath;
    std::string     g_licenseFileName("license.txt");
    std::string     g_licenseFilePath;
    std::string     g_defaultFontMap("Lucida Console");

}
