#include "opaquePass.if.hlsl"
// this is trying to be Vertex shader file.
struct VertexOut
{
  float2 uv : TEXCOORD0;
  float4 pos : SV_Position;
};


[RootSignature(ROOTSIG)]
VertexOut main(uint id: SV_VertexID)
{ 
  uint loadID = id * 3;

  const uint multiplier = 4;
  VertexOut vtxOut;
  float3 i1;
  i1 = asfloat(vertexInput.Load3(loadID*multiplier));
  vtxOut.pos.xyz = i1;
  vtxOut.pos.w = 1.f;
  //int outputVal;
  //debugPrint.InterlockedAdd(0, 1337, outputVal);
  if ( 0 && id % 2 == 1)
  {
    vtxOut.pos.z += 100.f;
  }
  vtxOut.pos = mul(vtxOut.pos, constants.worldMat);
  vtxOut.pos = mul(vtxOut.pos, constants.viewMat);
  vtxOut.uv.x = (id % 3 == 2) ?  1 : 0;
  vtxOut.uv.y = (id % 3 == 1) ?  1 : 0;
  return vtxOut;
}
