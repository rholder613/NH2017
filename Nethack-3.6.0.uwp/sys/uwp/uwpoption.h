/* NetHack 3.6	uwpoption.h	$NHDT-Date:  $  $NHDT-Branch:  $:$NHDT-Revision:  $ */
/* Copyright (c) Bart House, 2016-2017. */
/* Nethack for the Universal Windows Platform (UWP) */
/* NetHack may be freely redistributed.  See license for details. */
#pragma once

namespace Nethack
{
    class Options
    {
    public:

        bool Load(std::string & filePath);
        void Store();
        void RemoveOption(std::string & option);
        std::string GetString();
    
        std::vector<std::string> m_options;
        std::string m_filePath;
    };
}
