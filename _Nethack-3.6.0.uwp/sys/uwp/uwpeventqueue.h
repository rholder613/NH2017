/* NetHack 3.6	uwpeventqueue.h	$NHDT-Date:  $  $NHDT-Branch:  $:$NHDT-Revision:  $ */
/* Copyright (c) Bart House, 2016-2017. */
/* Nethack for the Universal Windows Platform (UWP) */
/* NetHack may be freely redistributed.  See license for details. */
#pragma once

namespace Nethack
{
        
    class Event
    {
    public:

        enum class Type { Undefined, Char, Mouse, ScanCode };
        enum class Tap { Undefined, Left, Right };

        Event()
        {
            m_type = Type::Undefined;
        }

        Event(char inChar)
        {
            m_type = Type::Char;
            m_char = inChar;
        }

        Event(const Int2D & pos, Tap tap = Tap::Left)
        {
            m_type = Type::Mouse;
            m_tap = tap;
            m_pos = pos;
        }

        Event(ScanCode scanCode, bool shift, bool control, bool alt = false)
        {
            m_type = Type::ScanCode;
            m_scanCode = scanCode;
            m_shift = shift;
            m_control = control;
            m_alt = alt;
        }

        Type m_type;

        Tap m_tap;
        Int2D m_pos;

        char m_char;

        ScanCode m_scanCode;

        bool m_shift;
        bool m_control;
        bool m_alt;

    };

    class EventQueue
    {
    public:

        EventQueue()
        {
            // do nothing
        }

        ~EventQueue()
        {
            // do nothing
        }

        void PushBack(const Event & inEvent)
        {
            m_lock.AcquireExclusive();
            m_list.push_back(inEvent);
            m_lock.ReleaseExclusive();
            m_conditionVariable.Wake();
        }

#if 0
        void PushFront(const Event & inEvent)
        {
            m_lock.AcquireExclusive();
            m_list.push_front(inEvent);
            m_lock.ReleaseExclusive();
            m_conditionVariable.Wake();
        }
#endif

        Event PopFront()
        {
            m_lock.AcquireExclusive();
            while (m_list.empty())
                m_conditionVariable.Sleep(m_lock);
            Event e = m_list.front();
            m_list.pop_front();
            m_lock.ReleaseExclusive();

            return e;
        }

        bool Empty()
        {
            m_lock.AcquireExclusive();
            bool empty = m_list.empty();
            m_lock.ReleaseExclusive();
            return empty;
        }

    private:
        
        std::list<Event>    m_list;
        Lock                m_lock;
        ConditionVariable   m_conditionVariable;

    };
    
}