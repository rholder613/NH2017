/* NetHack 3.6	uwpconditionvariable.h	$NHDT-Date:  $  $NHDT-Branch:  $:$NHDT-Revision:  $ */
/* Copyright (c) Bart House, 2016-2017. */
/* Nethack for the Universal Windows Platform (UWP) */
/* NetHack may be freely redistributed.  See license for details. */
#pragma once

namespace Nethack
{

    class ConditionVariable
    {
    public:

        ConditionVariable()
        {
            InitializeConditionVariable(&m_conditionVariable);
        }

        void Sleep(Lock & inLock)
        {
            inLock.m_ownerThreadId = 0;
            BOOL success = SleepConditionVariableSRW(&m_conditionVariable, &inLock.m_lock, INFINITE, 0);
            inLock.m_ownerThreadId = GetCurrentThreadId();
            assert(success);
        }

        void Wake()
        {
            WakeConditionVariable(&m_conditionVariable);
        }

    private:

        CONDITION_VARIABLE m_conditionVariable;
    };

}
