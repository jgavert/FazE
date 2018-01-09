#ifndef POSTEFFECT_HLSL
#define POSTEFFECT_HLSL

#include "app/graphics/definitions.hpp"

FAZE_BEGIN_LAYOUT(PostEffect)

FAZE_CBUFFER
{
  uint2 sourceRes;
  uint2 targetRes;
};

FAZE_SRV(Texture2D<float4>, source, 0)
FAZE_UAV(RWTexture2D<float4>, target, 0)

FAZE_SRV_TABLE(1)
FAZE_UAV_TABLE(1)

FAZE_END_LAYOUT

#endif /*FAZE_SHADER_DEFINITIONS*/