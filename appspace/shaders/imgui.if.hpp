#ifndef IMGUI_HLSL
#define IMGUI_HLSL

#include "definitions.hpp"

FAZE_BEGIN_LAYOUT(ImGui)

FAZE_BEGIN_DESCRIPTOR_LAYOUT
FAZE_BYTEADDRESSBUFFERS(1)
FAZE_TEXTURES(1)
FAZE_END_DESCRIPTOR_LAYOUT

FAZE_CBUFFER
{
  float2 reciprocalResolution;
};

FAZE_SRV(ByteAddressBuffer, vertices, 0)
FAZE_SRV(Texture2D<float>, tex, 1)

FAZE_SAMPLER_POINT(pointSampler)

FAZE_END_LAYOUT

#endif /*FAZE_SHADER_DEFINITIONS*/