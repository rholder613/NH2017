/* NetHack 3.6	uwpconf.h	$NHDT-Date:  $  $NHDT-Branch:  $:$NHDT-Revision:  $ */
/* Copyright (c) Bart House, 2016-2017. */
/* Nethack for the Universal Windows Platform (UWP) */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef UWPCONF_H
#define UWPCONF_H

#ifndef UWP
#error UWP is expected to be defined at this point
#endif

#ifndef TTY_GRAPHICS
#error TTY_GRAPHICS is expected to be defined at this point
#endif

#ifndef DEFAULT_WINDOW_SYS
#error DEFAULT_WINDOW_SYS is expecte to be set and equal to "tty"
#endif

#ifdef MSWIN_GRAPHICS
#error MSWIN_GRAPHICS is not expected to be defined
#endif

#define RANDOM    /* have Berkeley random(3) */
#define TEXTCOLOR /* Color text */
#define CHANGE_COLOR /* allow modifications to color palette */

#define EXEPATH              /* Allow .exe location to be used as HACKDIR */
#define TRADITIONAL_GLYPHMAP /* Store glyph mappings at level change time */

#define PC_LOCKING /* Prevent overwrites of aborted or in-progress games */
/* without first receiving confirmation. */

#define HOLD_LOCKFILE_OPEN /* Keep an exclusive lock on the .0 file */

#define SELF_RECOVER /* Allow the game itself to recover from an aborted \
                        game */


#define SELECTSAVED /* Provide menu of saved games to choose from at start \
                       */

/*
 * -----------------------------------------------------------------
 *  The remaining code shouldn't need modification.
 * -----------------------------------------------------------------
 */

#ifdef MICRO
#undef MICRO /* never define this! */
#endif

#define NOCWD_ASSUMPTIONS /* Always define this. There are assumptions that \
                             it is defined for WIN32.                       \
                             Allow paths to be specified for HACKDIR,       \
                             LEVELDIR, SAVEDIR, BONESDIR, DATADIR,          \
                             SCOREDIR, LOCKDIR, CONFIGDIR, and TROUBLEDIR */
#define NO_TERMS
#define ASCIIGRAPH

#ifdef OPTIONS_USED
#undef OPTIONS_USED
#endif
#define OPTIONS_USED "options"
#define OPTIONS_FILE OPTIONS_USED

#define PORT_HELP "porthelp"

#define SAFERHANGUP /* Define SAFERHANGUP to delay hangup processing   \
                     * until the main command loop. 'safer' because it \
                     * avoids certain cheats and also avoids losing    \
                     * objects being thrown when the hangup occurs.    \
                     */

 /*
 * 3.4.3 addition - Stuff to help the user with some common, yet significant
 * errors
 * Let's make it NOP for now
 */
#define interject_assistance(_1, _2, _3, _4)
#define interject(_1)

/*
 *===============================================
 * Compiler-specific adjustments
 *===============================================
 */
#ifdef _MSC_VER
#if (_MSC_VER > 1000)
/* Visual C 8 warning elimination */
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif
#ifndef _SCL_SECURE_NO_DEPRECATE
#define _SCL_SECURE_NO_DEPRECATE
#endif
#ifndef _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE
#endif
#pragma warning(disable : 4996) /* VC8 deprecation warnings */
#pragma warning(disable : 4142) /* benign redefinition */
#pragma warning(disable : 4267) /* conversion from 'size_t' to XX */
#if (_MSC_VER > 1600)
#pragma warning(disable : 4459) /* hide global declaration */
#endif                          /* _MSC_VER > 1600 */
#endif                          /* _MSC_VER > 1000 */
#pragma warning(disable : 4761) /* integral size mismatch in arg; conv \
                                   supp*/
#ifdef YYPREFIX
#pragma warning(disable : 4102) /* unreferenced label */
#endif
#ifdef __cplusplus
/* suppress a warning in cppregex.cpp */
#pragma warning(disable : 4101) /* unreferenced local variable */
#endif
#endif /* _MSC_VER */


/* The following is needed for prototypes of certain functions */
#if defined(_MSC_VER)
#include <process.h> /* Provides prototypes of exit(), spawn()      */
#endif

#include <string.h> /* Provides prototypes of strncmpi(), etc.     */
#ifdef STRNCMPI
#define strncmpi(a, b, c) strnicmp(a, b, c)
#endif

/* Visual Studio defines this in their own headers, which we don't use */
#ifndef snprintf
#define snprintf _snprintf
#pragma warning( \
    disable : 4996) /* deprecation warning suggesting snprintf_s */
#endif

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#ifdef __BORLANDC__
#undef randomize
#undef random
#endif

#define PATHLEN BUFSZ  /* maximum pathlength */
#define FILENAME BUFSZ /* maximum filename length (conservative) */

#if defined(_MAX_PATH) && defined(_MAX_FNAME)
#if (_MAX_PATH < BUFSZ) && (_MAX_FNAME < BUFSZ)
#undef PATHLEN
#undef FILENAME
#define PATHLEN _MAX_PATH
#define FILENAME _MAX_FNAME
#endif
#endif

#define NO_SIGNAL
#define index strchr
#define rindex strrchr

/* Time stuff */
#include <time.h>

#define USE_STDARG
#ifdef RANDOM
/* Use the high quality random number routines. */
#define Rand() random()
#else
#define Rand() rand()
#endif

#include <sys/stat.h>
#define FCMASK (_S_IREAD | _S_IWRITE) /* file creation mask */
#define regularize nt_regularize
#define HLOCK "NHPERM"

#ifndef M
#define M(c) ((char) (0x80 | (c)))
/* #define M(c)		((c) - 128) */
#endif

#ifndef C
#define C(c) (0x1f & (c))
#endif

#if defined(DLB)
#define FILENAME_CMP stricmp /* case insensitive */
#endif

/* this was part of the MICRO stuff in the past */
extern const char *alllevels, *allbones;
#define ABORT C('a')
#define getuid() 1
#define getlogin() ((char *) 0)
extern void NDECL(win32_abort);
extern void FDECL(nttty_preference_update, (const char *));
extern void NDECL(toggle_mouse_support);
extern void FDECL(map_subkeyvalue, (char *));
extern void NDECL(load_keyboard_handler);
extern void NDECL(raw_clear_screen);

#include <fcntl.h>

#include <io.h>
#include <direct.h>
#include <conio.h>

extern int NDECL(kbhit);

#ifdef LAN_FEATURES
#define MAX_LAN_USERNAME 20
#endif

#ifndef alloca
#define ALLOCA_HACK /* used in util/panic.c */
#endif

extern int FDECL(set_win32_option, (const char *, const char *));
#define LEFTBUTTON FROM_LEFT_1ST_BUTTON_PRESSED
#define RIGHTBUTTON RIGHTMOST_BUTTON_PRESSED
#define MIDBUTTON FROM_LEFT_2ND_BUTTON_PRESSED
#define MOUSEMASK (LEFTBUTTON | RIGHTBUTTON | MIDBUTTON)
#ifdef CHANGE_COLOR
extern int FDECL(alternative_palette, (char *));
#endif

/* Windows declares terminate */
#define terminate xxxterminate

/* Include window headers that conflict with nethack macros */
#include <windows.h>

extern char * uwp_getenv(const char * env);
#define getenv(x) uwp_getenv(x)

/* No SYSCF support */
#undef SYSCF
#undef SYSCF_FILE

/* No NEWS support */
#undef NEWS

#endif /* UWPCONF_H */
