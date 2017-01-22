/* NetHack 3.6	uwputil.h	$NHDT-Date:  $  $NHDT-Branch:  $:$NHDT-Revision:  $ */
/* Copyright (c) Bart House, 2016-2017. */
/* Nethack for the Universal Windows Platform (UWP) */
/* NetHack may be freely redistributed.  See license for details. */
#pragma once

namespace Nethack
{
    inline std::string to_string(const std::wstring & inString)
    {
        std::string outString;
        outString.assign(inString.begin(), inString.end());
        return outString;
    }

    inline void to_wstring(const std::string & inString, std::wstring & outString)
    {
        outString.assign(inString.begin(), inString.end());
    }

    inline std::wstring to_wstring(const std::string & inString)
    {
        std::wstring outString;
        outString.assign(inString.begin(), inString.end());
        return outString;
    }

    inline void to_string(const std::wstring & inString, std::string & outString)
    {
        outString.assign(inString.begin(), inString.end());
    }

    inline Platform::String ^ to_platform_string(const std::string & inString)
    {
        std::wstring string;
        string.assign(inString.begin(), inString.end());
        return ref new Platform::String(string.c_str());
    }

    inline Platform::String ^ to_platform_string(const std::wstring & inString)
    {
        return ref new Platform::String(inString.c_str());
    }

    inline std::string to_string(Platform::String ^ inString)
    {
        std::wstring string(inString->Data());
        std::string outString;
        outString.assign(string.begin(), string.end());
        return outString;
    }

    inline std::wstring to_wstring(Platform::String ^ inString)
    {
        return std::wstring(inString->Data());
    }

}

