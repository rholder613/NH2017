/* NetHack 3.6	uwplock.h	$NHDT-Date:  $  $NHDT-Branch:  $:$NHDT-Revision:  $ */
/* Copyright (c) Bart House, 2016-2017. */
/* Nethack for the Universal Windows Platform (UWP) */
/* NetHack may be freely redistributed.  See license for details. */
#pragma once

namespace Nethack
{

    class ConditionVariable;

    class Lock
    {
    public:

        Lock(void);
        ~Lock(void);

        void AcquireExclusive(void);
        void ReleaseExclusive(void);
        bool HasExclusive(void);

    private:

        friend class ConditionVariable;

        SRWLOCK m_lock;
        DWORD m_ownerThreadId;
    };

}
