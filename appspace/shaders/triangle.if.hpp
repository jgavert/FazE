#ifndef TRIANGLE_HLSL
#define TRIANGLE_HLSL

#include "definitions.hpp"

FAZE_BEGIN_LAYOUT(Triangle)

FAZE_CBUFFER
{
  float4 color;
  int colorspace;
};

FAZE_SRV(Buffer<float4>, vertices, 0)
FAZE_SRV(Texture2D<float4>, yellow, 1)

FAZE_SAMPLER_POINT(staSam)

FAZE_SRV_TABLE(2)
FAZE_UAV_TABLE(0)

FAZE_END_LAYOUT

#endif /*FAZE_SHADER_DEFINITIONS*/