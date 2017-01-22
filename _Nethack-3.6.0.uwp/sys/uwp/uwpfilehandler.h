/* NetHack 3.6	uwpfilehandler.h	$NHDT-Date:  $  $NHDT-Branch:  $:$NHDT-Revision:  $ */
/* Copyright (c) Bart House, 2016-2017. */
/* Nethack for the Universal Windows Platform (UWP) */
/* NetHack may be freely redistributed.  See license for details. */
#pragma once

namespace Nethack
{

    ref class FileHandler sealed
    {
    public:

        static property FileHandler ^ s_instance;

        FileHandler();

        void SetCoreDispatcher(Windows::UI::Core::CoreDispatcher ^ coreDispatcher)
        {
            m_coreDispatcher = coreDispatcher;
        }

        void SaveFilePicker(Platform::String ^ fileText, Platform::String ^ fileName, Platform::String ^ extension);
        Platform::String ^ LoadFilePicker(Platform::String ^ extension);

    private:

        Windows::UI::Core::CoreDispatcher ^ m_coreDispatcher;
        Nethack::Lock m_lock;
        Nethack::ConditionVariable m_conditionVariable;
        Windows::Storage::StorageFile ^ m_file;

    };

}

