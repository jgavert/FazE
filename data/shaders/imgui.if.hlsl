// INTERFACE_HASH:14276129400140057606:16335188131946678899
// This file is generated from code.
#ifdef HIGANBANA_VULKAN
#define VK_BINDING(index, set) [[vk::binding(index, set)]]
#else // HIGANBANA_DX12
#define VK_BINDING(index, set) 
#endif

#define ROOTSIG "RootFlags(0), \
  CBV(b0), \
    DescriptorTable(UAV(u99, numDescriptors = 1, space=99 )),\
DescriptorTable(\
     SRV(t0, numDescriptors = 2, space=0 )),\
  StaticSampler(s0, filter = FILTER_MIN_MAG_LINEAR_MIP_POINT), \
  StaticSampler(s1, filter = FILTER_MIN_MAG_MIP_POINT), \
  StaticSampler(s2, filter = FILTER_MIN_MAG_LINEAR_MIP_POINT, addressU = TEXTURE_ADDRESS_WRAP, addressV = TEXTURE_ADDRESS_WRAP, addressW = TEXTURE_ADDRESS_WRAP), \
  StaticSampler(s3, filter = FILTER_MIN_MAG_MIP_POINT, addressU = TEXTURE_ADDRESS_WRAP, addressV = TEXTURE_ADDRESS_WRAP, addressW = TEXTURE_ADDRESS_WRAP)"

struct Constants
{float2 reciprocalResolution; };
VK_BINDING(0, 1) ConstantBuffer<Constants> constants : register( b0 );
VK_BINDING(1, 1) RWByteAddressBuffer debugPrint : register( u99, space99 );
// Shader Arguments 0

// Read Only resources
VK_BINDING(0, 0) ByteAddressBuffer vertices : register( t0, space0 );
VK_BINDING(1, 0) Texture2D<float> tex : register( t1, space0 );

// Read Write resources

// Usable Static Samplers
VK_BINDING(2, 1) SamplerState bilinearSampler : register( s0 );
VK_BINDING(3, 1) SamplerState pointSampler : register( s1 );
VK_BINDING(4, 1) SamplerState bilinearSamplerWarp : register( s2 );
VK_BINDING(5, 1) SamplerState pointSamplerWrap : register( s3 );
