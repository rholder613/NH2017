/* NetHack 3.6	uwp.cpp	$NHDT-Date:  $  $NHDT-Branch:  $:$NHDT-Revision:  $ */
/* Copyright (c) Bart House, 2016-2017. */
/* Nethack for the Universal Windows Platform (UWP) */
/* NetHack may be freely redistributed.  See license for details. */
#include "uwp.h"

using namespace Nethack;

extern "C"  {

#ifndef HANGUPHANDLING
#error HANGUPHANDLING must be defined
#endif

#ifdef PC_LOCKING
    static int
        eraseoldlocks()
    {
        register int i;

        /* cannot use maxledgerno() here, because we need to find a lock name
        * before starting everything (including the dungeon initialization
        * that sets astral_level, needed for maxledgerno()) up
        */
        for (i = 1; i <= MAXDUNGEON * MAXLEVEL + 1; i++) {
            /* try to remove all */
            set_levelfile_name(lock, i);
            (void)unlink(fqname(lock, LEVELPREFIX, 0));
        }
        set_levelfile_name(lock, 0);
#ifdef HOLD_LOCKFILE_OPEN
        really_close();
#endif
        if (unlink(fqname(lock, LEVELPREFIX, 0)))
            return 0; /* cannot remove it */
        return (1);   /* success! */
    }

    void
        getlock()
    {
        register int fd, ern;
        int fcmask = FCMASK;
        char tbuf[BUFSZ];
        const char *fq_lock;

        // Shoudl only be called when windowing system is initialized
        assert(iflags.window_inited);

        /* we ignore QUIT and INT at this point */
        if (!lock_file(HLOCK, LOCKPREFIX, 10)) {
            wait_synch();
            error("Quitting.");
        }

        /* regularize(lock); */ /* already done in uwpmains */
        Sprintf(tbuf, "%s", fqname(lock, LEVELPREFIX, 0));
        set_levelfile_name(lock, 0);
        fq_lock = fqname(lock, LEVELPREFIX, 1);

        if ((fd = open(fq_lock, 0)) == -1) {

            if (errno == ENOENT)
                goto gotlock; /* no such file */

            unlock_file(HLOCK);
            error("Unexpected failure.  Cannot open %s", fq_lock);
        }

        (void)nhclose(fd);

        if (yn("There are files from a game in progress under your name. Recover?")) {
            if (recover_savefile()) {
                goto gotlock;
            }
            else if (yn("Recovery failed.  Continue by removing corrupt saved files?")) {
                if (!eraseoldlocks()) {
                    unlock_file(HLOCK);
                    error("Cound not remove corrupt saved files.  Exiting.");
                }
                goto gotlock;
            }
            else {
                unlock_file(HLOCK);
                error("Can not continue.  Exiting.");
            }
        }
        else {
            unlock_file(HLOCK);
            error("Cannot start a new game.");
        }

    gotlock:
        fd = creat(fq_lock, fcmask);

        if (fd == -1)
            ern = errno;

        unlock_file(HLOCK);

        if (fd == -1) {
            error("Unexpected error creating file (%s).", fq_lock);
        }
        else {
            if (write(fd, (char *)&hackpid, sizeof(hackpid)) != sizeof(hackpid)) {
                error("Unexpected error writing to file (%s)", fq_lock);
            }
            if (nhclose(fd) == -1) {
                error("Cannot close file (%s)", fq_lock);
            }
        }
    }
#endif /* PC_LOCKING */

void
    verify_record_file()
{
    const char *fq_record;
    int fd;

    char tmp[PATHLEN];

    Strcpy(tmp, RECORD);
    fq_record = fqname(RECORD, SCOREPREFIX, 0);

    if ((fd = open(fq_record, O_RDWR)) < 0) {
        /* try to create empty record */
        if ((fd = open(fq_record, O_CREAT | O_RDWR, S_IREAD | S_IWRITE))
            < 0) {
            raw_printf("Warning: cannot write record %s", tmp);
            wait_synch();
        }
        else
            (void)nhclose(fd);
    }
    else /* open succeeded */
        (void)nhclose(fd);
}

void
nttty_open(int mode)
{
    LI = Nethack::g_textGrid.GetDimensions().m_y;
    CO = Nethack::g_textGrid.GetDimensions().m_x;
}

    
int kbhit(void)
{
    return !Nethack::g_eventQueue.Empty();
}

// used in panic
#if 1
void
win32_abort()
{
    // TODO(bhouse) Decide whether there is any value in retail NetHack
    //              having abilty to enter debug break.  For now, we
    //              will not allow it.

#if 0
    if (wizard) {
        int c, ci, ct;

        if (!iflags.window_inited)
            c = 'n';
        ct = 0;
        msmsg("Execute debug breakpoint wizard?");
        while ((ci = nhgetch()) != '\n') {
            if (ct > 0) {
                backsp(); /* \b is visible on NT */
                (void)putchar(' ');
                backsp();
                ct = 0;
                c = 'n';
            }
            if (ci == 'y' || ci == 'n' || ci == 'Y' || ci == 'N') {
                ct = 1;
                c = ci;
                msmsg("%c", c);
            }
        }
        if (c == 'y')
            __debugbreak();
    }
#endif

    abort();
}
#endif

/* caller should have cleared screen appropriately before calling exit 
   and the screen should have exit reason. */
void nethack_exit(int result)
{
    if(!program_state.done_hup) {
        if(iflags.window_inited) {
            putstr(BASE_WINDOW, 0, "Hit <ENTER> to exit.");
        } else {
            raw_printf("Hit <ENTER> to exit.");
        }

        while(pgetchar() != '\n')
            ;
    }

    longjmp(Nethack::g_mainLoopJmpBuf, -1);
}

HANDLE ffhandle = (HANDLE)0;
WIN32_FIND_DATAA ffd;

int findfirst(char *path)
{
    if (ffhandle) {
        FindClose(ffhandle);
        ffhandle = (HANDLE)0;
    }
    ffhandle = FindFirstFileExA(path, FindExInfoStandard, &ffd, FindExSearchNameMatch, NULL, 0);
    return (ffhandle == INVALID_HANDLE_VALUE) ? 0 : 1;
}

int findnext(void)
{
    return FindNextFileA(ffhandle, &ffd) ? 1 : 0;
}

char * foundfile_buffer()
{
    return &ffd.cFileName[0];
}

void Delay(int ms)
{
    Sleep(ms);
}

// TODO(bhouse): Dangerous call ... string needs to have been allocated with enough space.
//               Caller should pass in length of allocation.
void append_slash(char * name)
{
    char *ptr;

    if (!*name)
        return;

    ptr = name + (strlen(name) - 1);

    if (*ptr != '\\' && *ptr != '/' && *ptr != ':') {
        *++ptr = '\\';
        *++ptr = '\0';
    }
}

char erase_char, kill_char;

int
has_color(int color)
{
    if ((color >= 0) && (color < (int) Nethack::TextColor::Count))
        return 1;

    return 0;
}

#define MAX_OVERRIDES 256
unsigned char key_overrides[MAX_OVERRIDES];

void
map_subkeyvalue(char * op)
{
    char digits[] = "0123456789";
    int length, i, idx, val;
    char *kp;

    idx = -1;
    val = -1;
    kp = index(op, '/');
    if (kp) {
        *kp = '\0';
        kp++;
        length = strlen(kp);
        if (length < 1 || length > 3)
            return;
        for (i = 0; i < length; i++)
            if (!index(digits, kp[i]))
                return;
        val = atoi(kp);
        length = strlen(op);
        if (length < 1 || length > 3)
            return;
        for (i = 0; i < length; i++)
            if (!index(digits, op[i]))
                return;
        idx = atoi(op);
    }
    if (idx >= MAX_OVERRIDES || idx < 0 || val >= MAX_OVERRIDES || val < 1)
        return;
    key_overrides[idx] = val;
}

// TODO: we have no keyboard handlers -- we should remove need to define
void
load_keyboard_handler()
{ 
    return;
}

void
nttty_preference_update(const char * pref)
{
    // do nothing
}

char MapScanCode(const Nethack::Event & e)
{
    assert(e.m_type == Nethack::Event::Type::ScanCode);

    char c = 0;

    if (!e.m_alt) {
        if (e.m_scanCode >= Nethack::ScanCode::Home && e.m_scanCode <= Nethack::ScanCode::Delete) {
            typedef struct {
                char normal, shift, control;
            } PadMapping;

            static const PadMapping keypad[] =
            {
                { 'y', 'Y', C('y') },    /* 7 */
                { 'k', 'K', C('k') },    /* 8 */
                { 'u', 'U', C('u') },    /* 9 */
                { 'm', C('p'), C('p') }, /* - */
                { 'h', 'H', C('h') },    /* 4 */
                { 'g', 'G', 'g' },       /* 5 */
                { 'l', 'L', C('l') },    /* 6 */
                { '+', 'P', C('p') },    /* + */
                { 'b', 'B', C('b') },    /* 1 */
                { 'j', 'J', C('j') },    /* 2 */
                { 'n', 'N', C('n') },    /* 3 */
                { 'i', 'I', C('i') },    /* Ins */
                { '.', ':', ':' }        /* Del */
            };

            static const PadMapping numpad[] = {
                { '7', M('7'), '7' },    /* 7 */
                { '8', M('8'), '8' },    /* 8 */
                { '9', M('9'), '9' },    /* 9 */
                { 'm', C('p'), C('p') }, /* - */
                { '4', M('4'), '4' },    /* 4 */
                { '5', M('5'), '5' },    /* 5 */
                { '6', M('6'), '6' },    /* 6 */
                { '+', 'P', C('p') },    /* + */
                { '1', M('1'), '1' },    /* 1 */
                { '2', M('2'), '2' },    /* 2 */
                { '3', M('3'), '3' },    /* 3 */
                { '0', M('0'), '0' },    /* Ins */
                { '.', ':', ':' }        /* Del */
            };

            const PadMapping * pad = iflags.num_pad ? numpad : keypad;

            int i = (int) e.m_scanCode - (int)Nethack::ScanCode::Home;

            if (e.m_shift) {
                c = pad[i].shift;
            } else if (e.m_control) {
                c = pad[i].control;
            } else {
                c = pad[i].normal;
            }
        }
    } else {
        int scanToChar[(int)Nethack::ScanCode::Count] = {
            0, // Unknown
            0, // Escape
            '1' | 0x80, // One
            '2' | 0x80, // Two
            '3' | 0x80, // Three
            '4' | 0x80, // Four
            '5' | 0x80, // Five
            '6' | 0x80, // Six
            '7' | 0x80, // Seven
            '8' | 0x80, // Eight
            '9' | 0x80, // Nine
            '0' | 0x80, // Zero
            0, // Minus
            0, // Equal
            0, // Backspace
            0, // Tab
            'q' | 0x80, // Q
            'w' | 0x80, // W
            'e' | 0x80, // E
            'r' | 0x80, // R
            't' | 0x80, // T
            'y' | 0x80, // Y
            'u' | 0x80, // U
            'i' | 0x80, // I
            'o' | 0x80, // O
            'p' | 0x80, // P
            0, // LeftBracket
            0, //  RightBracket
            0, // Enter
            0, // Control
            'a' | 0x80, // A
            's' | 0x80, // S
            'd' | 0x80, // D
            'f' | 0x80, // F
            'g' | 0x80, // G
            'h' | 0x80, // H
            'j' | 0x80, // J
            'k' | 0x80, // K
            'l' | 0x80, // L
            0, // SemiColon
            0, // Quote
            0, // BackQuote
            0, // LeftShift
            0, // BackSlash
            'z' | 0x80, // Z
            'x' | 0x80, // X
            'c' | 0x80, // C
            'v' | 0x80, // V
            'b' | 0x80, // B
            'n' | 0x80, // N
            'm' | 0x80, // M
            0, // Comma
            0, // Period
            '?' | 0x80, // ForwardSlash
            0, // RightShift
            0, // NotSure
            0, // Alt
            0, // Space
            0, // Caps
            0, // F1
            0, // F2
            0, // F3
            0, // F4
            0, // F5
            0, // F6
            0, // F7
            0, // F8
            0, // F9
            0, // F10
            0, // Num
            0, // Scroll
            0, // Home
            0, // Up
            0, // PageUp
            0, // PadMinus
            0, // Left
            0, // Center
            0, // Right
            0, // PadPlus
            0, // End
            0, // Down
            0, // PageDown
            0, // Insert
            0, // Delete
            0, // Scan84
            0, // Scan85
            0, // Scan86
            0, // F11
            0, // F12
        };

        assert(((int) e.m_scanCode >= 0) && ((int) e.m_scanCode < (int) Nethack::ScanCode::Count));

        if (e.m_scanCode >= Nethack::ScanCode::Unknown && e.m_scanCode < Nethack::ScanCode::Count) {
            c = (char) scanToChar[(int)e.m_scanCode];
            unsigned char uc = c & ~0x80;
            if (e.m_shift && isalpha(uc)) {
                c = toupper(uc) | 0x80;
            }
        }
        
    }

    return c;
}

int raw_getchar()
{
    if (program_state.done_hup)
        return ESCAPE;

    Nethack::Event e;

    while (e.m_type == Nethack::Event::Type::Undefined ||
           (e.m_type == Nethack::Event::Type::ScanCode && MapScanCode(e) == 0) ||
           (e.m_type == Nethack::Event::Type::Mouse)) {
        e = Nethack::g_eventQueue.PopFront();
    }

    if (e.m_type == Nethack::Event::Type::ScanCode)
        return MapScanCode(e);
    else  {
        assert(e.m_type == Nethack::Event::Type::Char);
        return e.m_char;
    }
}

void
gettty()
{
    erase_char = '\b';
    kill_char = 21; /* cntl-U */
    iflags.cbreak = TRUE;
}

void
setftty()
{
    start_screen();
}

#ifndef NO_MOUSE_ALLOWED
void
toggle_mouse_support()
{
    // TODO: toggle reporting mouse events
    return;
}
#endif

void error VA_DECL(const char *, s)
{
    char buf[BUFSZ];
    VA_START(s);
    VA_INIT(s, const char *);

    (void)vsprintf(buf, s, VA_ARGS);

    if(iflags.window_inited) {
        clear_nhwindow(BASE_WINDOW);
        putstr(BASE_WINDOW, 0, buf);
    } else {
        raw_clear_screen();
        raw_printf(buf);
    }

    VA_END();

    nethack_exit(EXIT_FAILURE);
}

// TODO(bhouse) Do we already have a warn somewhere else called something else?
void warn VA_DECL(const char *, s)
{
    char buf[BUFSZ];
    VA_START(s);
    VA_INIT(s, const char *);

    (void)vsprintf(buf, s, VA_ARGS);

    if (iflags.window_inited) {
        clear_nhwindow(BASE_WINDOW);
        putstr(BASE_WINDOW, 0, buf);
        putstr(BASE_WINDOW, 0, "Hit <ENTER> to exit.");
    }
    else {
        raw_clear_screen();
        raw_printf(buf);
        raw_printf("Hit <ENTER> to exit.");
    }

    VA_END();

    while (pgetchar() != '\n')
        ;
}

void raw_clear_screen(void)
{
    Nethack::g_textGrid.Clear();
}

bool get_string(std::string & string, unsigned int maxLength)
{
    string = "";

    bool done = false;
    while (1) {
        char c = raw_getchar();

        if (c == EOF || c == ESCAPE) return false;

        if (c == '\n') return true;

        if (c == '\b') {
            if (string.length() > 0) {
                Nethack::g_textGrid.Putstr(Nethack::TextColor::White, Nethack::TextAttribute::None, "\b");
                string = string.substr(0, string.length() - 1);
            }
            continue;
        }

        if (!isprint(c))
            continue;

        if (string.length() < maxLength)  {
            Nethack::g_textGrid.Put(Nethack::TextColor::White, Nethack::TextAttribute::None, c);
            string += c;
        }
    }
}

bool file_exists(std::string & filePath)
{
    std::ifstream f(filePath.c_str());
    return f.good();
}

void rename_file(const char * from, const char * to)
{
    std::string toPath(to);

    if (file_exists(toPath))
        return;

    rename(from, to);
}

void copy_to_local(std::string & fileName, bool onlyIfMissing)
{
    std::string localPath = Nethack::g_localDir;
    localPath += fileName;

    std::string installPath = Nethack::g_installDir;
    installPath += fileName;

    if (onlyIfMissing && file_exists(localPath))
        return;

    FILE * dstFp = fopen(localPath.c_str(), "wb+");
    assert(dstFp != NULL);

    if(dstFp != NULL) {
        FILE * srcFp = fopen(installPath.c_str(), "rb");
        assert(srcFp != NULL);
        if(srcFp != NULL) {
            int failure = fseek(srcFp, 0, SEEK_END);
            assert(!failure);
            if(!failure) {
                size_t fileSize = ftell(srcFp);
                rewind(srcFp);
                char * data = (char *)malloc(fileSize);
                assert(data != NULL);
                if(data != NULL) {
                    int readBytes = fread(data, 1, fileSize, srcFp);
                    assert(readBytes == fileSize);
                    if(readBytes == fileSize) {
                        int writeBytes = fwrite(data, 1, fileSize, dstFp);
                        assert(writeBytes == fileSize);
                    }
                    free(data);
                }
            }
            fclose(srcFp);
        }
        fclose(dstFp);
    }
}

bool validate_font_map(std::string & fontFamilyName)
{
    bool goodGood = false;

    if (g_fontCollection.m_fontFamilies.count(fontFamilyName) != 0)
    {
        auto & font = g_fontCollection.m_fontFamilies[fontFamilyName];
        if (!font.m_fonts.begin()->second.m_monospaced) {
            warn("font_map font '%s' must be monospaced.", fontFamilyName.c_str());
        } else {
            goodGood = true;
        }
    }
    else {
        warn("unable to find font_map font '%s'", fontFamilyName.c_str());
    }
    return goodGood;
}

void process_font_map()
{
    std::string fontFamilyName;

    if (iflags.wc_font_map) {
        fontFamilyName = std::string(iflags.wc_font_map);
        if (!validate_font_map(fontFamilyName))
            fontFamilyName = g_defaultFontMap;
    } else {
        fontFamilyName = g_defaultFontMap;
    }

    g_textGrid.SetFontFamilyName(fontFamilyName);
}


void add_option()
{
    clear_nhwindow(BASE_WINDOW);

    char buf[BUFSZ];
    getlin("Option:", buf);

    if(!buf[0]) return;

    // TODO(bhouse) It would be better if validateoptons caused event
    //              for setting font
    std::string option(buf);
    if (option.compare(0, 9, "font_map:") == 0) {
        std::string fontFamilyName = option.substr(9, std::string::npos);
        if (!validate_font_map(fontFamilyName))
            return;
    }

    if(validateoptions(buf, FALSE)) {
        Nethack::g_options.m_options.push_back(option);
        Nethack::g_options.Store();
        g_textGrid.SetPaletteDefault();
        initoptions();
        process_font_map();
    }
}

void remove_options()
{
    clear_nhwindow(BASE_WINDOW);

    winid menu = create_nhwindow(NHW_MENU);
    start_menu(menu);

    anything any = zeroany;

    add_menu(menu, NO_GLYPH, &any, 0, 0, ATR_NONE, "NETHACKOPTIONS", MENU_UNSELECTED);
    add_menu(menu, NO_GLYPH, &any, 0, 0, ATR_NONE, "", MENU_UNSELECTED);

    for(size_t i = 0; i < g_options.m_options.size(); i++) {
        auto & option = g_options.m_options[i];
        any.a_int = i + 1;
        add_menu(menu, NO_GLYPH, &any, 0, 0, ATR_NONE, option.c_str(), MENU_UNSELECTED);
    }

    end_menu(menu, "Pick options to remove");

    menu_item *picks = NULL;
    int count = select_menu(menu, PICK_ANY, &picks);
    destroy_nhwindow(menu);

    std::vector<bool> removals(g_options.m_options.size(), false);
    for(int i = 0; i < count; i++)
        removals[picks[i].item.a_int-1] = true;

    for(int i = g_options.m_options.size() - 1; i >= 0; i--)
        if(removals[i]) g_options.m_options.erase(g_options.m_options.begin() + i);

    if(count > 0) {
        Nethack::g_options.Store();
        g_textGrid.SetPaletteDefault();
        initoptions();
        process_font_map();
    }

}

void change_options()
{
    bool done = false;
    while (!done) {

        clear_nhwindow(BASE_WINDOW);

        std::string optionsString = "[ ]";
        optionsString += Nethack::g_options.GetString();

        winid menu = create_nhwindow(NHW_MENU);
        start_menu(menu);

        anything any = zeroany;

        for(auto & option : g_options.m_options) {
            add_menu(menu, NO_GLYPH, &any, 0, 0, ATR_NONE, option.c_str(), MENU_UNSELECTED);
        }

        const char * items[] = {
            "[ ]",
            "[a]Add Options",
            "[b]Remove Options",
        };

        for (int i = 0; i < SIZE(items); i++) {
            any.a_int = items[i][1] == ' ' ? 0 : items[i][1];
            add_menu(menu, NO_GLYPH, &any, 0, 0, ATR_NONE, &items[i][3], MENU_UNSELECTED);
        }

        end_menu(menu, "Change NETHACKOPTIONS");

        menu_item *pick = NULL;
        int count = select_menu(menu, PICK_ONE, &pick);
        destroy_nhwindow(menu);

        if (count == 1) {
            switch(pick->item.a_int) {
            case 'a': add_option(); break;
            case'b': remove_options(); break;
            }
        } else       
            done = true;

        free((genericptr_t)pick);
    }
}

void change_font(void)
{
    clear_nhwindow(BASE_WINDOW);

    winid menu = create_nhwindow(NHW_MENU);
    start_menu(menu);

    anything any = zeroany;

    for(auto & fontFamily : g_fontCollection.m_fontFamilies) {
        if (fontFamily.second.m_fonts.begin()->second.m_monospaced) {
            any.a_void = (void *)&fontFamily.first;
            add_menu(menu, NO_GLYPH, &any, 0, 0, ATR_NONE, fontFamily.first.c_str(), MENU_UNSELECTED);
        }
    }

    end_menu(menu, "Pick font");

    menu_item *pick = NULL;
    int count = select_menu(menu, PICK_ONE, &pick);
    destroy_nhwindow(menu);

    if(count == 1) {
        std::string fontFamilyName = *((std::string *) pick->item.a_void);

        // remove any options for font_map
        g_options.RemoveOption(std::string("font_map"));

        // add font_map option for picked font
        std::string option = "font_map:";
        option += fontFamilyName;
        g_options.m_options.push_back(option);
        g_options.Store();

        // set font into grid
        g_textGrid.SetFontFamilyName(fontFamilyName);
    }

    free((genericptr_t)pick);
}

char * uwp_getenv(const char * env)
{
    if(strcmp(env, "NETHACKOPTIONS") == 0) {
        static std::string options;
        options = Nethack::g_options.GetString();
        return (char *)options.c_str();
    }
    return NULL;
}

void save_file(std::string & filePath)
{
    std::string fileExtension = filePath.substr(filePath.rfind('.'), std::string::npos);
    std::string fileName = filePath.substr(filePath.rfind('\\') + 1, filePath.rfind('.') - filePath.rfind('\\') - 1);

    std::string readText;
    std::fstream input(filePath.c_str(), std::fstream::in | std::fstream::binary);
    if(input.is_open()) {
        input.seekg(0, std::ios::end);
        size_t size = (size_t)input.tellg();
        input.seekg(0, std::ios::beg);
        std::vector<char> bytes(size);
        input.read(bytes.data(), size);
        input.close();
        readText = std::string(bytes.data(), size);

        Platform::String ^ fileText = Nethack::to_platform_string(readText);
        Platform::String ^ fileNameStr = Nethack::to_platform_string(fileName);
        Platform::String ^ extensionStr = Nethack::to_platform_string(fileExtension);
        Nethack::FileHandler::s_instance->SaveFilePicker(fileText, fileNameStr, extensionStr);
    } else {
        error("Unable to open %s%s", fileName.c_str(), fileExtension.c_str());
    }

}

void load_file(std::string & filePath)
{
    std::string fileExtension = filePath.substr(filePath.rfind('.'), std::string::npos);
    Platform::String ^ extensionStr = Nethack::to_platform_string(fileExtension);
    Platform::String ^ fileText = Nethack::FileHandler::s_instance->LoadFilePicker(extensionStr);
    if (fileText != nullptr)  {
        std::string writeText = Nethack::to_string(fileText);
        std::fstream output(filePath.c_str(), std::fstream::out | std::fstream::trunc | std::fstream::binary);
        if (output.is_open())  {
            output.write(writeText.c_str(), writeText.length());
            output.close();
        }
    }
}

void reset_defaults_file(void)
{
    copy_to_local(g_defaultsFileName, false);
    clear_nhwindow(BASE_WINDOW);
    getreturn("Reset complete");
}

bool main_menu(void)
{
    // TODO(bhouse): Need to review use of BASE_WINDOW.  Really like to have this code be
    //               windowing system agnostic.  Use and knowledge of BASE_WINDOW breaks
    //               that goal.
    clear_nhwindow(BASE_WINDOW);

    bool play = false;
    bool done = false;
    while (!done && !play) {
        // TODO(bhouse): this can be removed when we switch to using nh menus for all actions
        Nethack::g_textGrid.Clear();

        const char * items[] = {
            "[ ]" COPYRIGHT_BANNER_A,
            "[ ]" COPYRIGHT_BANNER_B,
            "[ ]Universal Windows Port, Copyright 2016-2017",
            "[ ]         By Bart House",
            "[ ]" COPYRIGHT_BANNER_C,
            "[ ]" COPYRIGHT_BANNER_D,
            "[ ]",
            "[ ]Source: https://github.com/barthouse/NetHackPublic",
            "[ ]Support: https://github.com/barthouse/NetHackPublic/wiki/Support",
            "[ ]Email: nethack@barthouse.com",
            "[ ]",
            "[ ]Pick an action",
            "[ ]",
            "[ ]",
            "[a]Play",
            "[ ]",
            "[b]Save Guidebook.txt to file",
            "[c]Save License.txt to file",
            "[d]Change font",
            "[e]Change NETHACKOPTIONS",
            "[f]Save defaults.nh to file",
            "[g]Load defaults.nh from file",
            "[h]Reset defaults.nh"
        };

        winid menu = create_nhwindow(NHW_MENU);
        start_menu(menu);

        anything any = zeroany;

        for (int i = 0; i < SIZE(items); i++) {
            any.a_int = items[i][1] == ' ' ? 0 : items[i][1];
            add_menu(menu, NO_GLYPH, &any, 0, 0, ATR_NONE, &items[i][3], MENU_UNSELECTED);
        }

        end_menu(menu, (char *)0);

        menu_item *pick = NULL;
        int count = select_menu(menu, PICK_ONE, &pick);
        destroy_nhwindow(menu);

        if (count == 1) {
            switch (pick->item.a_int) {
            case 'a': play = true; break;
            case 'b': save_file(g_guidebookFilePath); break;
            case 'c': save_file(g_licenseFilePath); break;
            case 'd': change_font(); break;
            case 'e': change_options(); break;
            case 'f': save_file(g_defaultsFilePath); break;
            case 'g': load_file(g_defaultsFilePath); initoptions();  break;
            case 'h': reset_defaults_file(); break;
            }
        } else if (count == -1) {
            done = true;
        }

        free((genericptr_t)pick);
    }

    return play;
}

extern void rename_file(const char * from, const char * to);

/* to support previous releases, we rename save games to new save game format */
void rename_save_files()
{
    char *foundfile;
    const char *fq_save;

    const char * oldsaves[] = { 
        "bhouse-*.NetHack-saved-game",
        "noname-*.NetHack-saved-game" };

    for (int i = 0; i < SIZE(oldsaves); i++) {
        fq_save = fqname(oldsaves[i], SAVEPREFIX, 0);

        foundfile = foundfile_buffer();
        if (findfirst((char *)fq_save)) {
            do {
                char oldPath[512];
                char newname[512];

                strcpy(newname, &foundfile[7]);

                fq_save = fqname(foundfile, SAVEPREFIX, 0);
                strcpy(oldPath, fq_save);

                const char * newPath = fqname(newname, SAVEPREFIX, 0);

                rename_file(oldPath, newPath);

            } while (findnext());
        }
    }
}

extern boolean uwpmain(void);
extern void decl_clean_up(void);

void mainloop(const char * localDir, const char * installDir)
{
    /* first things first ... we jump buffer which is where we will jump to
       if we exit.  We assume anythign can fail so we do this first. */
    if(setjmp(Nethack::g_mainLoopJmpBuf) == 0) {

        first_init();

        /* next thing we do is set the window system so that raw output will
           function correctly thorough the window proc */
        choose_windows(DEFAULT_WINDOW_SYS);

        hname = "NetHack"; /* used for syntax messages */

        g_localDir = std::string(localDir);
        g_localDir += "\\";

        g_installDir = std::string(installDir);
        g_installDir += "\\";

        fqn_prefix[HACKPREFIX] = (char *) g_installDir.c_str();
        fqn_prefix[LEVELPREFIX] = (char *) g_localDir.c_str();
        fqn_prefix[SAVEPREFIX] = (char *)g_localDir.c_str();
        fqn_prefix[BONESPREFIX] = (char *)g_localDir.c_str();
        fqn_prefix[DATAPREFIX] = (char *) g_installDir.c_str();
        fqn_prefix[SCOREPREFIX] = (char *)g_localDir.c_str();
        fqn_prefix[LOCKPREFIX] = (char *)g_localDir.c_str();
        fqn_prefix[SYSCONFPREFIX] = (char *) g_installDir.c_str();
        fqn_prefix[CONFIGPREFIX] = (char *)g_localDir.c_str();
        fqn_prefix[TROUBLEPREFIX] = (char *)g_localDir.c_str();

        copy_to_local(g_defaultsFileName, true);
        copy_to_local(g_guidebookFileName, true);
        copy_to_local(g_licenseFileName, true);
        rename_save_files();

        g_nethackOptionsFilePath = g_localDir;
        g_nethackOptionsFilePath += g_nethackOptionsFileName;

        g_defaultsFilePath = Nethack::g_localDir;
        g_defaultsFilePath += g_defaultsFileName;

        g_guidebookFilePath = Nethack::g_installDir;
        g_guidebookFilePath += g_guidebookFileName;

        g_licenseFilePath = Nethack::g_installDir;
        g_licenseFilePath += g_licenseFileName;

        Nethack::g_options.Load(g_nethackOptionsFilePath);

        g_textGrid.SetPaletteDefault();
        initoptions();
        process_font_map();

        int argc = 1;
        char * argv[1] = { "nethack" };

        init_nhwindows(&argc, argv);

        display_gamewindows();

        assert(iflags.window_inited);
        if(!iflags.window_inited) {
            error("Default windowing system failed to initialize");
        }

        bool bPlay = main_menu();

        exit_nhwindows((char *)0);

        if (bPlay) {

            sys_early_init();

            char failbuf[BUFSZ];
            if (!validate_prefix_locations(failbuf)) {
                error("Some invalid directory locations were specified:\n\t%s",
                    failbuf);
            }

            if(!dlb_init()) {
                error("dlb_init failure");
            }

            boolean resuming = uwpmain();

            moveloop(resuming);
        }
    }

    final_cleanup();
}



boolean
uwpmain(void)
{
    int fd;
    char *envp = NULL;
    char *sptr = NULL;

    char fnamebuf[BUFSZ], encodedfnamebuf[BUFSZ];
    boolean resuming = FALSE; /* assume new game */

    u.uhp = 1; /* prevent RIP on early quits */
    u.ux = 0;  /* prevent flush_screen() */

    verify_record_file();

    /* In 3.6.0, several ports process options before they init
    * the window port. This allows settings that impact window
    * ports to be specified or read from the sys or user config files.
    */

    iflags.use_background_glyph = FALSE;
    nttty_open(1);

    /* Player didn't specify any symbol set so use IBM defaults */
    if (!symset[PRIMARY].name) {
        load_symset("IBMGraphics_2", PRIMARY);
    }
    if (!symset[ROGUESET].name) {
        load_symset("RogueEpyx", ROGUESET);
    }

    int argc = 1;
    char * argv[1] = { "nethack" };
    init_nhwindows(&argc, argv);

    toggle_mouse_support(); /* must come after process_options */

                            // load up the game windows before we start asking questions
    display_gamewindows();

    /* strip role,race,&c suffix; calls askname() if plname[] is empty
    or holds a generic user name like "player" or "games" */
    plnamesuffix();

    set_playmode(); /* sets plname to "wizard" for wizard mode */

                    /* unlike Unix where the game might be invoked with a script
                    which forces a particular character name for each player
                    using a shared account, we always allow player to rename
                    the character during role/race/&c selection */
    iflags.renameallowed = TRUE;

#if defined(PC_LOCKING)
    /* 3.3.0 added this to support detection of multiple games
    * under the same plname on the same machine in a windowed
    * or multitasking environment.
    *
    * That allows user confirmation prior to overwriting the
    * level files of a game in progress.
    *
    * Also prevents an aborted game's level files from being
    * overwritten without confirmation when a user starts up
    * another game with the same player name.
    */
    /* Obtain the name of the logged on user and incorporate
    * it into the name. */
    Sprintf(fnamebuf, "%s", plname);
    (void)fname_encode(
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_-.", '%',
        fnamebuf, encodedfnamebuf, BUFSZ);
    Sprintf(lock, "%s", encodedfnamebuf);
    getlock();
#endif /* PC_LOCKING */

    /* Set up level 0 file to keep the game state.
    */
    fd = create_levelfile(0, (char *)0);
    if (fd < 0) {
        /* TODO(bhouse) do we need to pring anything here?
        commenting out for now.  Either we have an error and
        can't continue or we just forge ahead. */
        /* raw_print("Cannot create lock file"); */
    }
    else {
        hackpid = GetCurrentProcessId();
        write(fd, (genericptr_t)&hackpid, sizeof(hackpid));
        nhclose(fd);
    }

    /*
    *  Initialize the vision system.  This must be before mklev() on a
    *  new game or before a level restore on a saved game.
    */
    vision_init();

    /*
    * First, try to find and restore a save file for specified character.
    * We'll return here if new game player_selection() renames the hero.
    */
attempt_restore:
    if ((fd = restore_saved_game()) >= 0) {
        pline("Restoring save file...");
        mark_synch(); /* flush output */

        if (dorecover(fd)) {
            resuming = TRUE; /* not starting new game */
            if (discover)
                You("are in non-scoring discovery mode.");
            if (discover || wizard) {
                if (yn("Do you want to keep the save file?") == 'n')
                    (void) delete_savefile();
                else {
                    nh_compress(fqname(SAVEF, SAVEPREFIX, 0));
                }
            }
        }
    }

    if (!resuming) {
        /* new game:  start by choosing role, race, etc;
        player might change the hero's name while doing that,
        in which case we try to restore under the new name
        and skip selection this time if that didn't succeed */
        if (!iflags.renameinprogress) {
            player_selection();
            if (iflags.renameinprogress) {
                /* player has renamed the hero while selecting role;
                discard current lock file and create another for
                the new character name */
                delete_levelfile(0); /* remove empty lock file */
                getlock();
                goto attempt_restore;
            }
        }
        newgame();
        if (discover)
            You("are in non-scoring discovery mode.");
    }

    return resuming;
}

#ifdef PORT_HELP
void
port_help()
{
    /* display port specific help file */
    display_file(PORT_HELP, 1);
}
#endif /* PORT_HELP */

/* validate wizard mode if player has requested access to it */
boolean
authorize_wizard_mode()
{
    if (!strcmp(plname, WIZARD_NAME))
        return TRUE;
    return FALSE;
}

} // extern "C"
