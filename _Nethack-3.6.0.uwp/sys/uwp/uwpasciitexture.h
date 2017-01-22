/* NetHack 3.6	uwpasciitexture.h	$NHDT-Date:  $  $NHDT-Branch:  $:$NHDT-Revision:  $ */
/* Copyright (c) Bart House, 2016-2017. */
/* Nethack for the Universal Windows Platform (UWP) */
/* NetHack may be freely redistributed.  See license for details. */
#pragma once

namespace DX
{
    class DeviceResources;
}

namespace Nethack
{

    class AsciiTexture
    {
    public:

        AsciiTexture(DX::DeviceResources * deviceResources) : m_deviceResources(deviceResources) { }
        void Create(const std::string & fontName, DWRITE_FONT_WEIGHT weight, float lineHeight);

        void GetGlyphRect(unsigned char c, Nethack::FloatRect & outRect) const;

        std::string                                         m_fontFamilyName;
        float                                               m_fontHeight;
        float                                               m_lineHeight;

        Microsoft::WRL::ComPtr<ID3D11Texture2D>             m_newTexture;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>    m_newTextureShaderResourceView;
        Microsoft::WRL::ComPtr<ID3D11SamplerState>          m_asciiTextureSampler;
        Int2D                                               m_glyphPixels;
        float                                               m_underlinePosition;  // relative to top of alignment box in pixels
        float                                               m_underlineThickness; // in pixels
        bool                                                m_autoGen;

    private:

        DX::DeviceResources * m_deviceResources;

    };

}