#include "simpleEffect.if.hlsl"
// this is trying to be Compute shader file.


[RootSignature(ROOTSIG)]
[numthreads(FAZE_THREADGROUP_X, FAZE_THREADGROUP_Y, FAZE_THREADGROUP_Z)] // @nolint
void main(uint2 id : SV_DispatchThreadID, uint2 gid : SV_GroupThreadID)
{ 
  const float sx = float(id.x) / constants.resx; 
  const float sy = float(id.y) / constants.resy; 
  output[id] = float4(sx, sy, 0.f, 1.f);
}