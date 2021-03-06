#pragma once

namespace Nethack
{
    const char ESCAPE = 27;

    enum class ScanCode
    {
        Unknown,
        Escape,
        One,
        Two,
        Three,
        Four,
        Five,
        Six,
        Seven,
        Eight,
        Nine,
        Zero,
        Minus,
        Equal,
        Backspace,
        Tab,
        Q,
        W,
        E,
        R,
        T,
        Y,
        U,
        I,
        O,
        P,
        LeftBracket,
        RightBracket,
        Enter,
        Control,
        A,
        S,
        D,
        F,
        G,
        H,
        J,
        K,
        L,
        SemiColon,
        Quote,
        BackQuote,
        LeftShift,
        BackSlash,
        Z,
        X,
        C,
        V,
        B,
        N,
        M,
        Comma,
        Period,
        ForwardSlash,
        RightShift,
        NotSure,
        Alt,
        Space,
        Caps,
        F1,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        Num,
        Scroll,
        Home,
        Up,
        PageUp,
        PadMinus,
        Left,
        Center,
        Right,
        PadPlus,
        End,
        Down,
        PageDown,
        Insert,
        Delete,
        Scan84,
        Scan85,
        Scan86,
        F11,
        F12,
        Count

    };

    enum class VirtualKey
    {
        Zero = 0x30,
        One,
        Two,
        Three,
        Four,
        Five,
        Six,
        Seven,
        Eight,
        Nine,

        A = 0x41,
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
        J,
        K,
        L,
        M,
        N,
        O,
        P,
        Q,
        R,
        S,
        T,
        U,
        V,
        W,
        X,
        Y,
        Z
    };

}
