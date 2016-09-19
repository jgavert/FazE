#include "shader_defines.hpp"

FAZE_BEGIN_LAYOUT(SampleShader)
struct DataFormat
{
  float element;
};
struct Constants
{
  float something[16];
};
FAZE_PushConstants(asdfg)
{
  int member1;
  float member2;
} pConstants;
FAZE_CBUFFER(Constants);
FAZE_BufferSRV(buffer, DataFormat, dataIn, 1, blaa);
FAZE_BufferUAV(buffer, DataFormat, dataOut, 2, bloo);
FAZE_DescriptorSetLayout(1, 2, 0, 0);
FAZE_END_LAYOUT