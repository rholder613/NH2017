/* NetHack 3.6	uwpcolor.cpp	$NHDT-Date:  $  $NHDT-Branch:  $:$NHDT-Revision:  $ */
/* Copyright (c) Bart House, 2016-2017. */
/* Nethack for the Universal Windows Platform (UWP) */
/* NetHack may be freely redistributed.  See license for details. */
#include "uwp.h"

using namespace Nethack;

extern "C"  {

    static int FDECL(match_color_name, (const char *));

    void
        tty_change_color(int color_number, long rgb, int reverse)
    {
        unsigned char red = rgb & 0xff;
        unsigned char green = (rgb >> 8) & 0xff;
        unsigned char blue = (rgb >> 8) & 0xff;

        assert(color_number >= 0 && color_number < (int)TextColor::Count);
        g_textGrid.SetPalette(color_number, red, green, blue);
    }

    char *
        tty_get_color_string()
    {
        return "";
    }

    int
        match_color_name(const char *c)
    {
        const struct others {
            int idx;
            const char *colorname;
        } othernames[] = {
            { CLR_MAGENTA, "purple" },
            { CLR_BRIGHT_MAGENTA, "bright purple" },
            { NO_COLOR, "dark gray" },
            { NO_COLOR, "dark grey" },
            { CLR_GRAY, "grey" },
        };

        int cnt;
        for (cnt = 0; cnt < CLR_MAX; ++cnt) {
            if (!strcmpi(c, c_obj_colors[cnt]))
                return cnt;
        }
        for (cnt = 0; cnt < SIZE(othernames); ++cnt) {
            if (!strcmpi(c, othernames[cnt].colorname))
                return othernames[cnt].idx;
        }
        return -1;
    }

    /*
    * Returns 0 if badoption syntax
    */
    int
        alternative_palette(char * op)
    {
        /*
        *	palette:color-R-G-B
        *	OPTIONS=palette:green-4-3-1
        */
        int fieldcnt, color_number, rgb, red, green, blue;
        char *fields[4], *cp;

        if (!op) {
            change_color(-1, 0, 0); /* indicates palette option with
                                    no value meaning "load an entire
                                    hard-coded NetHack palette." */
            return 1;
        }

        cp = fields[0] = op;
        for (fieldcnt = 1; fieldcnt < 4; ++fieldcnt) {
            cp = index(cp, '-');
            if (!cp)
                return 0;
            fields[fieldcnt] = cp;
            cp++;
        }
        for (fieldcnt = 1; fieldcnt < 4; ++fieldcnt) {
            *(fields[fieldcnt]) = '\0';
            ++fields[fieldcnt];
        }
        rgb = 0;
        for (fieldcnt = 0; fieldcnt < 4; ++fieldcnt) {
            if (fieldcnt == 0 && isalpha(*(fields[0]))) {
                color_number = match_color_name(fields[0]);
                if (color_number == -1)
                    return 0;
            }
            else {
                int dcount = 0, cval = 0;
                cp = fields[fieldcnt];
                if (*cp == '\\' && index("0123456789xXoO", cp[1])) {
                    const char *dp, *hex = "00112233445566778899aAbBcCdDeEfF";

                    cp++;
                    if (*cp == 'x' || *cp == 'X')
                        for (++cp; (dp = index(hex, *cp)) && (dcount++ < 2); cp++)
                            cval = (int)((cval * 16) + (dp - hex) / 2);
                    else if (*cp == 'o' || *cp == 'O')
                        for (++cp; (index("01234567", *cp)) && (dcount++ < 3);
                        cp++)
                            cval = (cval * 8) + (*cp - '0');
                    else
                        return 0;
                }
                else {
                    for (; *cp && (index("0123456789", *cp)) && (dcount++ < 3);
                        cp++)
                        cval = (cval * 10) + (*cp - '0');
                }
                switch (fieldcnt) {
                case 0:
                    color_number = cval;
                    break;
                case 1:
                    red = cval;
                    break;
                case 2:
                    green = cval;
                    break;
                case 3:
                    blue = cval;
                    break;
                }
            }
        }

        rgb = red | green << 8 | blue << 16;

        if (color_number >= 0 && color_number < CLR_MAX)
            change_color(color_number, rgb, 0);

        return 1;
    }


} // extern "C"
