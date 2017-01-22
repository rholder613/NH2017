#pragma once

#include "MyMath.h"

namespace NethackDx
{
    class ScreenPos : public Float2D
    {
    public:

    };

    class ScreenRect
    {
    public:

        ScreenRect(const ScreenPos & inTopLeft, const ScreenPos & inBottomRight)
            : m_topLeft(inTopLeft), m_bottomRight(inBottomRight)
        {
        }

        ScreenPos m_topLeft;
        ScreenPos m_bottomRight;

    };
}
