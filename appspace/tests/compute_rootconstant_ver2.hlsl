#include "tests/rootSig.hlsl"

struct buffer
{
  uint i;
  uint k;
  uint x;
  uint y;
};

ConstantBuffer<RootConstants> rootconst : register(b1, space0);
RWStructuredBuffer<buffer> Outd[] : register( u1 );

[RootSignature(MyRS4)]
[numthreads(32, 1, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
	int uv = DTid.x;
	if (uv != 0)
		return; 
	buffer buf;
	buf.x = 0;
	buf.y = 0;
	buf.i = uv;
	buf.k = rootconst.texIndex;
	Outd[rootconst.texIndex][0] = buf;
}
