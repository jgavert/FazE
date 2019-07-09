// INTERFACE_HASH:6853467395701554343:1861824900297809431
// This file is reflected from code.
#ifdef HIGANBANA_VULKAN
#define VK_BINDING(index) [[vk::binding(index)]]
#else // HIGANBANA_DX12
#define VK_BINDING(index) 
#endif

#define ROOTSIG "RootFlags(0), \
  CBV(b0), \
  DescriptorTable(\
     SRV(t0, numDescriptors = 2)),\
  StaticSampler(s0, filter = FILTER_MIN_MAG_LINEAR_MIP_POINT), \
  StaticSampler(s1, filter = FILTER_MIN_MAG_MIP_POINT), \
  StaticSampler(s2, filter = FILTER_MIN_MAG_LINEAR_MIP_POINT, addressU = TEXTURE_ADDRESS_WRAP, addressV = TEXTURE_ADDRESS_WRAP, addressW = TEXTURE_ADDRESS_WRAP), \
  StaticSampler(s3, filter = FILTER_MIN_MAG_MIP_POINT, addressU = TEXTURE_ADDRESS_WRAP, addressV = TEXTURE_ADDRESS_WRAP, addressW = TEXTURE_ADDRESS_WRAP)"

struct Constants
{float2 reciprocalResolution; };
VK_BINDING(0) ConstantBuffer<Constants> constants : register( b0 );

// Read Only resources
VK_BINDING(1) ByteAddressBuffer vertices : register( t0 );
VK_BINDING(2) Texture2D<float> tex : register( t1 );

// ReadWrite resources

// Usable Static Samplers
VK_BINDING(3) SamplerState bilinearSampler : register( s0 );
VK_BINDING(4) SamplerState pointSampler : register( s1 );
VK_BINDING(5) SamplerState bilinearSamplerWarp : register( s2 );
VK_BINDING(6) SamplerState pointSamplerWrap : register( s3 );