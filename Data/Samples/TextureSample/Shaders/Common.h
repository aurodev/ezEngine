#pragma once

#include "SampleConstantBuffer.h"
#include <Shaders/Common/GlobalConstants.h>

#if EZ_ENABLED(PLATFORM_SHADER)

struct VS_IN
{
  float3 Position : POSITION;
  float2 TexCoord0 : TEXCOORD0;
};

struct VS_OUT
{
  float4 Position : SV_Position;
  float2 TexCoord0 : TEXCOORD0;
};

typedef VS_OUT PS_IN;

#endif
