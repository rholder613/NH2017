/* NetHack 3.6	uwpfont.cpp	$NHDT-Date:  $  $NHDT-Branch:  $:$NHDT-Revision:  $ */
/* Copyright (c) Bart House, 2016-2017. */
/* Nethack for the Universal Windows Platform (UWP) */
/* NetHack may be freely redistributed.  See license for details. */
#include "uwp.h"

namespace Nethack
{
    Font::Font(const Microsoft::WRL::ComPtr<IDWriteFont3> & font, const std::string & name)
    {
        m_font = font;
        m_name = name;

        DX::ThrowIfFailed(m_font->CreateFontFace(&m_fontFace));

        m_monospaced = (font->IsMonospacedFont() != 0);

        m_font->GetMetrics(&m_metrics);

        m_lineSpacingEm = (float) (m_metrics.ascent + m_metrics.descent + m_metrics.lineGap) / (float) m_metrics.designUnitsPerEm;
        m_underlinePositionEm = (float) (m_metrics.ascent - m_metrics.underlinePosition) / (float) m_metrics.designUnitsPerEm;
        m_underlineThicknessEm = (float)m_metrics.underlineThickness / (float)m_metrics.designUnitsPerEm;

        CalculateMetrics();

        m_maxBoxTopEm = (float)m_maxBoxTop / (float)m_metrics.designUnitsPerEm;
        m_minBoxBottomEm = (float)m_minBoxBottom / (float)m_metrics.designUnitsPerEm;
        m_maxBoxWidthEm = (float)m_maxBoxWidth / (float)m_metrics.designUnitsPerEm;

        m_boxSizeEm.m_x = m_maxBoxWidthEm;
        m_boxSizeEm.m_y = m_maxBoxTopEm - m_minBoxBottomEm;

    }

    void Font::CalculateMetrics(void)
    {
        UINT32 usc4CodeSet[kGlyphCount] = { 219, '@', 'j' };

        DX::ThrowIfFailed(m_fontFace->GetGlyphIndices(usc4CodeSet, kGlyphCount, m_glyphIndices));
        DX::ThrowIfFailed(m_fontFace->GetDesignGlyphMetrics(m_glyphIndices, kGlyphCount, m_glyphMetrics, false));

        m_maxBoxWidth = 0;
        m_maxBoxTop = 0;
        m_minBoxBottom = 0;

        for(auto & metrics : m_glyphMetrics) {

            int boxWidth = metrics.advanceWidth - metrics.leftSideBearing - metrics.rightSideBearing;
            int boxHeight = metrics.advanceHeight - metrics.bottomSideBearing - metrics.topSideBearing;

            int boxTop = metrics.verticalOriginY - metrics.topSideBearing;
            int boxBottom = boxTop - boxHeight;

            m_maxBoxWidth = max(m_maxBoxWidth, boxWidth);
            m_maxBoxTop = max(m_maxBoxTop, boxTop);
            m_minBoxBottom = min(m_minBoxBottom, boxBottom);
        }
    }

    FontFamily::FontFamily(const Microsoft::WRL::ComPtr<IDWriteFontFamily1> & fontFamily, const std::string & name)
    {
        m_fontFamily = fontFamily;
        m_name = name;

        static bool found = false;
        if (name.compare("Arial") == 0)
        {
            found = true;
        }

        int fontCount = fontFamily->GetFontCount();

        for (int fontIndex = 0; fontIndex < fontCount; fontIndex++)
        {
            Microsoft::WRL::ComPtr<IDWriteFont3>	font;
            DX::ThrowIfFailed(fontFamily->GetFont(fontIndex, &font));

            Microsoft::WRL::ComPtr<IDWriteLocalizedStrings>	names;
            DX::ThrowIfFailed(font->GetFaceNames(&names));

            UINT32 nameIndex;
            BOOL exists;
            DX::ThrowIfFailed(names->FindLocaleName(L"en-us", &nameIndex, &exists));

            if (exists)
            {
                UINT32 length;
                DX::ThrowIfFailed(names->GetStringLength(nameIndex, &length));

                wchar_t * wname = new wchar_t[length + 1];
                DX::ThrowIfFailed(names->GetString(nameIndex, wname, length + 1));
                
                std::string name = Nethack::to_string(std::wstring(wname));
                m_fonts[name] = Font(font, name);
            }
        }
    }

    FontCollection::FontCollection(void)
    {

        // Initialize the DirectWrite Factory.
        DX::ThrowIfFailed(
            DWriteCreateFactory(
                DWRITE_FACTORY_TYPE_SHARED,
                __uuidof(IDWriteFactory3),
                &m_dwriteFactory
            )
        );

        DX::ThrowIfFailed(m_dwriteFactory->GetSystemFontCollection(false, &m_systemFontCollection));

        int familyCount = m_systemFontCollection->GetFontFamilyCount();
        for (int familyIndex = 0; familyIndex < familyCount; familyIndex++)
        {
            Microsoft::WRL::ComPtr<IDWriteFontFamily1>	fontFamily;
            DX::ThrowIfFailed(m_systemFontCollection->GetFontFamily(familyIndex, &fontFamily));

            Microsoft::WRL::ComPtr<IDWriteLocalizedStrings>	familyNames;
            DX::ThrowIfFailed(fontFamily->GetFamilyNames(&familyNames));

            UINT32 nameIndex;
            BOOL exists;
            DX::ThrowIfFailed(familyNames->FindLocaleName(L"en-us", &nameIndex, &exists));

            if (exists)
            {
                UINT32 familyNameLength;
                DX::ThrowIfFailed(familyNames->GetStringLength(nameIndex, &familyNameLength));

                wchar_t * wfamilyName = new wchar_t[familyNameLength + 1];
                DX::ThrowIfFailed(familyNames->GetString(nameIndex, wfamilyName, familyNameLength + 1));

                std::string familyName = Nethack::to_string(std::wstring(wfamilyName));
                m_fontFamilies[familyName] = FontFamily(fontFamily, familyName);
            }
        }
    }


}
