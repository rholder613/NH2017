/* NetHack 3.6	uwplock.cpp	$NHDT-Date:  $  $NHDT-Branch:  $:$NHDT-Revision:  $ */
/* Copyright (c) Bart House, 2016-2017. */
/* Nethack for the Universal Windows Platform (UWP) */
/* NetHack may be freely redistributed.  See license for details. */
#include "uwp.h"

namespace Nethack
{

    Lock::Lock(void)
    {
        InitializeSRWLock(&m_lock);
        m_ownerThreadId = 0;
    }

    Lock::~Lock(void)
    {
    }

    void Lock::AcquireExclusive(void)
    {
        assert(m_ownerThreadId != GetCurrentThreadId());
        AcquireSRWLockExclusive(&m_lock);
        m_ownerThreadId = GetCurrentThreadId();
    }

    void Lock::ReleaseExclusive(void)
    {
        assert(m_ownerThreadId == GetCurrentThreadId());
        m_ownerThreadId = 0;
        ReleaseSRWLockExclusive(&m_lock);
    }

    bool Lock::HasExclusive(void)
    {
        return (m_ownerThreadId == GetCurrentThreadId());
    }

}
