/* NetHack 3.6	uwppixelshader.hlsl	$NHDT-Date:  $  $NHDT-Branch:  $:$NHDT-Revision:  $ */
/* Copyright (c) Bart House, 2016-2017. */
/* Nethack for the Universal Windows Platform (UWP) */
/* NetHack may be freely redistributed.  See license for details. */

// Per-pixel color data passed through the pixel shader.
Texture2D AsciiTexture : register(t0);
SamplerState AsciiSampler : register(s0);

struct PixelShaderInput
{
    float4 pos : SV_POSITION;
    float3 foregroundColor : COLOR0;
    float3 backgroundColor : COLOR1;
    float2 tex : TEXCOORD0;
};

// A pass-through function for the (interpolated) color data.
float4 main(PixelShaderInput input) : SV_TARGET
{
    float4 alpha = AsciiTexture.Sample(AsciiSampler, input.tex);
    float4 inverseAlpha = float4(1.0f, 1.0f, 1.0f, 0.0f) - alpha;

    return (alpha * float4(input.foregroundColor, 0.0f)) + (inverseAlpha * float4(input.backgroundColor, 0.0f));
}
