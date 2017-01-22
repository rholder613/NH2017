/* NetHack 3.6	uwptty.cpp	$NHDT-Date:  $  $NHDT-Branch:  $:$NHDT-Revision:  $ */
/* Copyright (c) Bart House, 2016-2017. */
/* Nethack for the Universal Windows Platform (UWP) */
/* NetHack may be freely redistributed.  See license for details. */

/* Implement system port functions needed by TTY_GRAPHICS */

#include "uwp.h"

extern "C" {

#ifdef TTY_GRAPHICS

    // TODO: think through clear screen and its usage
    extern void NDECL(clear_screen);


    struct console_t {
        WORD background;
        WORD foreground;
        WORD attr;
        int current_nhcolor;
        WORD current_nhattr;
        COORD cursor;
    } console = {
        0,
        (FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED),
        (FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED),
        NO_COLOR,
        0,
        { 0, 0 }
    };

    void really_move_cursor()
    {

        if (ttyDisplay) {
            console.cursor.X = ttyDisplay->curx;
            console.cursor.Y = ttyDisplay->cury;
        }

        Nethack::g_textGrid.SetCursor(Nethack::Int2D(console.cursor.X, console.cursor.Y));
    }

    void getreturn(const char * str)
    {
        msmsg("Hit <Enter> %s.", str);

        while (pgetchar() != '\n')
            ;
        return;
    }

    /* this is used as a printf() replacement when the window
    * system isn't initialized yet
    */
    void msmsg
        VA_DECL(const char *, fmt)
    {
        char buf[ROWNO * COLNO]; /* worst case scenario */
        VA_START(fmt);
        VA_INIT(fmt, const char *);
        Vsprintf(buf, fmt, VA_ARGS);

        xputs(buf);

        if (ttyDisplay)
            curs(BASE_WINDOW, console.cursor.X + 1, console.cursor.Y);

        VA_END();
        return;
    }

    void tty_number_pad(int state)
    {
        // do nothing for now
    }

    void
        tty_startup(int * wid, int * hgt)
    {
        *wid = Nethack::g_textGrid.GetDimensions().m_x;
        *hgt = Nethack::g_textGrid.GetDimensions().m_y;

        set_option_mod_status("mouse_support", SET_IN_GAME);
    }

    void
        tty_start_screen()
    {
        if (iflags.num_pad)
            tty_number_pad(1); /* make keypad send digits */
    }

    void
        home()
    {
        console.cursor.X = console.cursor.Y = 0;
        ttyDisplay->curx = ttyDisplay->cury = 0;
    }

    void
        tty_nhbell()
    {
        // do nothing
    }

    void
        tty_end_screen()
    {
        clear_screen();
        really_move_cursor();
    }

    void
        cmov(int x, int y)
    {
        ttyDisplay->cury = y;
        ttyDisplay->curx = x;
        console.cursor.X = x;
        console.cursor.Y = y;
    }

    void
        nocmov(int x, int y)
    {
        console.cursor.X = x;
        console.cursor.Y = y;
        ttyDisplay->curx = x;
        ttyDisplay->cury = y;
    }

    void
        term_start_color(int color)
    {
        console.current_nhcolor = color;
    }

    void
        term_end_color(void)
    {
        console.current_nhcolor = NO_COLOR;
    }

    void
        term_end_raw_bold(void)
    {
        term_end_attr(ATR_BOLD);
    }

    void
        term_start_raw_bold(void)
    {
        term_start_attr(ATR_BOLD);
    }

    void
        term_start_attr(int attrib)
    {
        assert(console.current_nhattr == 0);
        if (attrib != ATR_NONE && console.current_nhattr == 0)
            console.current_nhattr |= 1 << attrib;
    }

    void
        term_end_attr(int attrib)
    {
        console.current_nhattr &= ~(1 << attrib);
        assert(console.current_nhattr == 0);
    }

    void
        cl_eos()
    {
        int x = ttyDisplay->curx;
        int y = ttyDisplay->cury;

        int cx = Nethack::g_textGrid.GetDimensions().m_x - x;
        int cy = (Nethack::g_textGrid.GetDimensions().m_y - (y + 1)) * Nethack::g_textGrid.GetDimensions().m_x;

        Nethack::g_textGrid.Put(x, y, Nethack::TextCell(), cx + cy);

        tty_curs(BASE_WINDOW, x + 1, y);
    }

    void
        cl_end()
    {
        int cx;
        console.cursor.X = ttyDisplay->curx;
        console.cursor.Y = ttyDisplay->cury;
        cx = Nethack::g_textGrid.GetDimensions().m_x - console.cursor.X;

        Nethack::g_textGrid.Put(console.cursor.X, console.cursor.Y, Nethack::TextCell(), cx);

        tty_curs(BASE_WINDOW, (int)ttyDisplay->curx + 1, (int)ttyDisplay->cury);
    }

    void clear_screen(void)
    {
        raw_clear_screen();
        home();
    }

    void
        standoutbeg()
    {
        term_start_attr(ATR_BOLD);
    }

    void
        standoutend()
    {
        term_end_attr(ATR_BOLD);
    }

    void
        g_putch(int in_ch)
    {
        console.cursor.X = ttyDisplay->curx;
        console.cursor.Y = ttyDisplay->cury;

        Nethack::TextCell textCell((Nethack::TextColor) console.current_nhcolor, (Nethack::TextAttribute) console.current_nhattr, in_ch);

        Nethack::g_textGrid.Put(console.cursor.X, console.cursor.Y, textCell, 1);
    }

    void
        xputc_core(char ch)
    {
        switch (ch) {
        case '\n':
            console.cursor.Y++;
            /* fall through */
        case '\r':
            console.cursor.X = 0;
            break;
        case '\b':
            console.cursor.X--;
            break;
        default:

            Nethack::TextCell textCell((Nethack::TextColor) console.current_nhcolor, (Nethack::TextAttribute) console.current_nhattr, ch);

            Nethack::g_textGrid.Put(console.cursor.X, console.cursor.Y, textCell, 1);

            console.cursor.X++;
        }
    }

    void xputc(char ch)
    {
        console.cursor.X = ttyDisplay->curx;
        console.cursor.Y = ttyDisplay->cury;
        xputc_core(ch);
        return;
    }

    void
        xputs(const char *s)
    {
        int k;
        int slen = strlen(s);

        if (ttyDisplay) {
            console.cursor.X = ttyDisplay->curx;
            console.cursor.Y = ttyDisplay->cury;
        }

        if (s) {
            for (k = 0; k < slen && s[k]; ++k)
                xputc_core(s[k]);
        }
    }

    void
        backsp()
    {
        console.cursor.X = ttyDisplay->curx;
        console.cursor.Y = ttyDisplay->cury;
        xputc_core('\b');
    }

    void
        tty_delay_output()
    {
        // Delay 50ms
        Sleep(50);
    }

    int
        tgetch()
    {
        really_move_cursor();
        char c = raw_getchar();

        if (c == EOF)
        {
            hangup(0);
            c = '\033';
        }

        return c;
    }

    void
        settty(const char *s)
    {
        cmov(ttyDisplay->curx, ttyDisplay->cury);
        end_screen();
        if (s)
            raw_print(s);
    }

    int
        ntposkey(int *x, int *y, int * mod)
    {
        really_move_cursor();

        if (program_state.done_hup)
            return '\033';

        Nethack::Event e;

        while (e.m_type == Nethack::Event::Type::Undefined ||
            (e.m_type == Nethack::Event::Type::Mouse && !iflags.wc_mouse_support) ||
            (e.m_type == Nethack::Event::Type::ScanCode && MapScanCode(e) == 0))
            e = Nethack::g_eventQueue.PopFront();

        if (e.m_type == Nethack::Event::Type::Char)
        {
            if (e.m_char == EOF)
            {
                hangup(0);
                e.m_char = '\033';
            }

            return e.m_char;
        }
        else if (e.m_type == Nethack::Event::Type::Mouse)
        {
            *x = e.m_pos.m_x;
            *y = e.m_pos.m_y;
            *mod = (e.m_tap == Nethack::Event::Tap::Left ? CLICK_1 : CLICK_2);

            return 0;
        }
        else
        {
            assert(e.m_type == Nethack::Event::Type::ScanCode);
            return MapScanCode(e);
        }
    }


#endif

}