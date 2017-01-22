/* NetHack 3.6	uwpasciitexture.cpp	$NHDT-Date:  $  $NHDT-Branch:  $:$NHDT-Revision:  $ */
/* Copyright (c) Bart House, 2016-2017. */
/* Nethack for the Universal Windows Platform (UWP) */
/* NetHack may be freely redistributed.  See license for details. */
#include "uwp.h"

using namespace D2D1;
using namespace DirectX;
using namespace Microsoft::WRL;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml::Controls;
using namespace Platform;

namespace Nethack
{
        
   void AsciiTexture::Create(
       const std::string & fontFamilyName, DWRITE_FONT_WEIGHT weight, float lineHeight)
    {
        static const int glyphRowCount = 16;
        static const int glyphColumnCount = 16;

        static const int gutter = 1;

        m_fontFamilyName = fontFamilyName;
        FontFamily & fontFamily = g_fontCollection.m_fontFamilies[fontFamilyName];
        Font & font = fontFamily.m_fonts.begin()->second;

        m_lineHeight = lineHeight;

        // line height is in dips
        // we want lineHeight == font.m_lineSpacingEm * fontSize

        float fontSize = lineHeight / font.m_lineSpacingEm;

        if (fontSize >= 72.0f) fontSize = 72.0f;
        else if (fontSize >= 36.0f) fontSize = 36.0f;
        else if (fontSize >= 28.0f) fontSize = 28.0f;
        else if (fontSize >= 24.0f) fontSize = 24.0f;
        else if (fontSize >= 20.0f) fontSize = 20.0f;
        else if (fontSize >= 18.0f) fontSize = 18.0f;
        else if (fontSize >= 16.0f) fontSize = 16.0f;
        else if (fontSize >= 14.0f) fontSize = 14.0f;
        else if (fontSize >= 12.0f) fontSize = 12.0f;
        else if (fontSize >= 10.0f) fontSize = 10.0f;
        else if (fontSize >= 8.0f) fontSize = 8.0f;
        else if (fontSize >= 7.0f) fontSize = 7.0f;
        else if (fontSize >= 6.0f) fontSize = 6.0f;
        else fontSize = 5.0f;

        // 1 EM is fontSize which is in dips.  To convert dips to pixels, pixels = dips * 96 / dpi
        const float dpi = 96.0f;

        if (font.m_monospaced) {
            m_glyphPixels.m_x = (int) ceil(((float)font.m_glyphMetrics[0].advanceWidth / (float)font.m_metrics.designUnitsPerEm) * fontSize * 96.0f / dpi);
        } else {
            m_glyphPixels.m_x = (int) ceil(font.m_boxSizeEm.m_x * fontSize * 96.0f / dpi);
        }

        m_glyphPixels.m_y = (int)ceil(font.m_lineSpacingEm * fontSize * 96.0f / dpi);

        m_underlinePosition = font.m_underlinePositionEm * fontSize * 96.0f / dpi;
        m_underlineThickness = font.m_underlineThicknessEm * fontSize * 96.0f / dpi;

        // glyphs have a one pixel wide gutter

        ID3D11Device3 * d3dDevice = m_deviceResources->GetD3DDevice();
                    
        int glyphWidth = m_glyphPixels.m_x + (2 * gutter);
        int glyphHeight = m_glyphPixels.m_y + (2 * gutter);

        int textureWidth = glyphWidth * glyphColumnCount;
        int textureHeight = glyphHeight * glyphRowCount;

        ID2D1DeviceContext2 * d2dContext = m_deviceResources->GetD2DDeviceContext();

        d2dContext->SetDpi(dpi, dpi);

        D2D1_BITMAP_PROPERTIES1 bitmapTargetProperties =
            D2D1::BitmapProperties1(
            D2D1_BITMAP_OPTIONS_TARGET,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
            dpi,
            dpi
            );

        D2D1_SIZE_U bitmapSize;
        bitmapSize.width = textureWidth;
        bitmapSize.height = textureHeight;

        ComPtr<ID2D1Bitmap1> bitmapTarget;

        DX::ThrowIfFailed(
            d2dContext->CreateBitmap(
                bitmapSize, NULL, 0, &bitmapTargetProperties, &bitmapTarget
            )
        );

        d2dContext->SetTarget(bitmapTarget.Get());

        ComPtr<ID2D1SolidColorBrush> whiteBrush;
        DX::ThrowIfFailed(
            d2dContext->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::White),
                &whiteBrush
            )
        );

        D2D1_SIZE_F renderTargetSize = d2dContext->GetSize();

        d2dContext->BeginDraw();

        d2dContext->SetTransform(D2D1::Matrix3x2F::Identity());

        d2dContext->Clear(D2D1::ColorF(D2D1::ColorF::Black));

        ComPtr<IDWriteTextFormat> textFormat;
        IDWriteFactory3 * dwriteFactory = m_deviceResources->GetDWriteFactory();
        std::wstring wFontFamilyName = Nethack::to_wstring(fontFamilyName);

        DX::ThrowIfFailed(
            dwriteFactory->CreateTextFormat(
                wFontFamilyName.c_str(),
                nullptr,
                weight,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                fontSize,
                L"en-US", // locale
                &textFormat
            )
        );

        HRESULT hr;

        // Center the text horizontally.
        DX::ThrowIfFailed(textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));

        // Center the text vertically.
        DX::ThrowIfFailed(
            textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR)
        );

        for (int y = 0; y < 16; y++)
        {
            for (int x = 0; x < 16; x++)
            {
                char c = (y * 16) + x;
                wchar_t wc;

                int result = MultiByteToWideChar(437, 0, &c, 1, &wc, 1);

                D2D1_RECT_F rect;
                
                rect = D2D1::RectF((float)((x * glyphWidth) + gutter), (float)((y * glyphHeight) + gutter),
                    (float)(((x + 1) * glyphWidth) - gutter), (float)(((y + 1) * glyphHeight) - gutter));

                d2dContext->DrawText(
                    &wc,
                    1,
                    textFormat.Get(),
                    rect,
                    whiteBrush.Get()
                );
            }
        }

        // We ignore D2DERR_RECREATE_TARGET here. This error indicates that the device
        // is lost. It will be handled during the next call to Present.
        hr = d2dContext->EndDraw();
        if (hr != D2DERR_RECREATE_TARGET)
        {
            DX::ThrowIfFailed(hr);
        }

        d2dContext->SetTarget(NULL);

        ComPtr<ID2D1Bitmap1> bitmapStaging;

        D2D1_BITMAP_PROPERTIES1 bitmapStagingProperties =
            D2D1::BitmapProperties1(
            D2D1_BITMAP_OPTIONS_CPU_READ | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
            dpi,
            dpi
            );

        DX::ThrowIfFailed(
            d2dContext->CreateBitmap(
            bitmapSize, NULL, 0, &bitmapStagingProperties, &bitmapStaging
            )
        );

        D2D1_POINT_2U dstPoint = { 0 , 0 };
        D2D1_RECT_U srcRect = { 0, 0, (UINT32) textureWidth, (UINT32) textureHeight };
        DX::ThrowIfFailed(bitmapStaging->CopyFromBitmap(&dstPoint, bitmapTarget.Get(), &srcRect));

        D2D1_MAPPED_RECT rect;
        DX::ThrowIfFailed(bitmapStaging->Map(D2D1_MAP_OPTIONS_READ, &rect));

        /* create texture */
        UINT formatSupport;
        DX::ThrowIfFailed(d3dDevice->CheckFormatSupport(DXGI_FORMAT_B8G8R8A8_UNORM, &formatSupport));

        m_autoGen = (formatSupport & D3D11_FORMAT_SUPPORT_MIP_AUTOGEN) != 0;

        CD3D11_TEXTURE2D_DESC newTextureDesc(
            DXGI_FORMAT_B8G8R8A8_UNORM,
            textureWidth,        // Width
            textureHeight,       // Height
            1,                   // Array Size
            (m_autoGen ? 0 : 1), // Mip Level
            D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET);

        if (m_autoGen) {
            newTextureDesc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
            newTextureDesc.MipLevels = 0;
        }

        D3D11_SUBRESOURCE_DATA data;

        data.pSysMem = rect.bits;
        data.SysMemPitch = rect.pitch;
        data.SysMemSlicePitch = 0;

        DX::ThrowIfFailed(
            d3dDevice->CreateTexture2D(
            &newTextureDesc,
            NULL,
            &m_newTexture
        )
        );

        auto * d3dDeviceContext = m_deviceResources->GetD3DDeviceContext();

        d3dDeviceContext->UpdateSubresource(m_newTexture.Get(), 0, nullptr, rect.bits, static_cast<UINT>(rect.pitch), static_cast<UINT>(rect.pitch * textureHeight));

        CD3D11_SHADER_RESOURCE_VIEW_DESC newResourceViewDesc(
            m_newTexture.Get(),
            D3D11_SRV_DIMENSION_TEXTURE2D,  // D3D11_SRV_DIMENSION
            DXGI_FORMAT_UNKNOWN,            // DXGI_FORMAT
            0,                              // most detailed Mip
            -1,                             // mip levels
            0,                              // first array slice
            -1                              // array size
        );

        DX::ThrowIfFailed(
            d3dDevice->CreateShaderResourceView(
            m_newTexture.Get(),
            &newResourceViewDesc,
            &m_newTextureShaderResourceView
        )
        );

        if (m_autoGen)
        {
            d3dDeviceContext->GenerateMips(m_newTextureShaderResourceView.Get());
        }

        DX::ThrowIfFailed(bitmapStaging->Unmap());

        D3D_FEATURE_LEVEL featureLevel = m_deviceResources->GetDeviceFeatureLevel();

        // create the sampler
        D3D11_SAMPLER_DESC samplerDescription;
        ZeroMemory(&samplerDescription, sizeof(D3D11_SAMPLER_DESC));
        samplerDescription.Filter = D3D11_FILTER_ANISOTROPIC;
        samplerDescription.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDescription.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDescription.MipLODBias = 0.0f;
        samplerDescription.MaxAnisotropy = featureLevel > D3D_FEATURE_LEVEL_9_1 ? 4 : 2;
        samplerDescription.ComparisonFunc = D3D11_COMPARISON_NEVER;
        samplerDescription.BorderColor[0] = 0.0f;
        samplerDescription.BorderColor[1] = 0.0f;
        samplerDescription.BorderColor[2] = 0.0f;
        samplerDescription.BorderColor[3] = 0.0f;
        // allow use of all mip levels
        samplerDescription.MinLOD = 0;
        samplerDescription.MaxLOD = D3D11_FLOAT32_MAX;

        DX::ThrowIfFailed(
            d3dDevice->CreateSamplerState(
                &samplerDescription,
                &m_asciiTextureSampler
            )
        );
    }

    void AsciiTexture::GetGlyphRect(unsigned char c, Nethack::FloatRect & outRect) const
    {
        int x = c & 0xf;
        int y = c >> 4;

        const Nethack::Int2D & glyphPixels = m_glyphPixels;

        float gutterX = (1.0f / 16.0f) / glyphPixels.m_x;
        float gutterY = (1.0f / 16.0f) / glyphPixels.m_y;

        outRect.m_topLeft.m_x = ((1.0f / 16.0f) * x) + gutterX;
        outRect.m_bottomRight.m_x = outRect.m_topLeft.m_x + (1.0f / 16.0f) - (2.0f * gutterX);

        outRect.m_topLeft.m_y = ((1.0f / 16.0f) * y) + gutterY;
        outRect.m_bottomRight.m_y = outRect.m_topLeft.m_y + (1.0f / 16.0f) - (2.0f * gutterY);
    }
}