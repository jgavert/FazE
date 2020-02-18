#include "blitter.hpp"
#include <higanbana/core/system/memview.hpp>
SHADER_STRUCT(BlitConstants,
  uint colorspace;
);
namespace app 
{
namespace renderer
{
Blitter::Blitter(higanbana::GpuGroup& device)
{
  using namespace higanbana;
  auto inputDataLayout = ShaderArgumentsLayoutDescriptor()
    .readOnly(ShaderResourceType::Buffer, "float4", "vertices")
    .readOnly(ShaderResourceType::Texture2D, "float4", "source");
  m_input = device.createShaderArgumentsLayout(inputDataLayout);

  PipelineInterfaceDescriptor instancePipeline = PipelineInterfaceDescriptor()
    .constants<BlitConstants>()
    .shaderArguments(0, m_input);

  auto pipelineDescriptor = GraphicsPipelineDescriptor()
    .setInterface(instancePipeline)
    .setVertexShader("blitter")
    .setPixelShader("blitter")
    .setPrimitiveTopology(PrimitiveTopology::Triangle)
    .setDepthStencil(DepthStencilDescriptor()
      .setDepthEnable(false));

  pipeline = device.createGraphicsPipeline(pipelineDescriptor);
  renderpass = device.createRenderpass();
}

void Blitter::beginRenderpass(higanbana::CommandGraphNode& node, higanbana::TextureRTV& target2){
  node.renderpass(renderpass, target2);
  target = target2;
}

void Blitter::blit(higanbana::GpuGroup& device, higanbana::CommandGraphNode& node, higanbana::TextureSRV& source, float2 topleft, float2 size)
{
  using namespace higanbana;
  higanbana::vector<float4> vertices;

  /*
  Draw quad

          ------
          |   /|
          |  / |
          | /  |
          |/___|

  like so.
  */

  float left = topleft.x;
  float top = topleft.y;
  float right = topleft.x + size.x; // -0.8f + 1.f
  float bottom = topleft.y - size.y; // 0.8f - 1.f

  vertices.push_back(float4{ left, bottom,  0.f, 0.f });
  vertices.push_back(float4{ left,  top,    0.f, 1.f });
  vertices.push_back(float4{ right, top,    1.f, 1.f });

  vertices.push_back(float4{ right, bottom, 1.f, 0.f });
  vertices.push_back(float4{ left,  bottom, 0.f, 0.f });
  vertices.push_back(float4{ right, top,    1.f, 1.f });

  auto verts = device.dynamicBuffer<float4>(vertices, higanbana::FormatType::Float32RGBA);

  auto args = device.createShaderArguments(ShaderArgumentsDescriptor("Opaque Arguments", m_input)
    .bind("vertices", verts)
    .bind("source", source));
  auto binding = node.bind(pipeline);
  binding.arguments(0, args);
  node.draw(binding, 6, 1);
}

void Blitter::blit(higanbana::GpuGroup& device, higanbana::CommandGraphNode& node, higanbana::TextureSRV& source, int2 itopleft, int2 isize)
{
  using namespace higanbana;
  float x = float(isize.x) / float(target.desc().desc.width);
  float y = float(isize.y) / float(target.desc().desc.height);

  float2 size = { x, y };

  x = float(itopleft.x) / float(target.desc().desc.width);
  y = float(itopleft.y) / float(target.desc().desc.height);

  float2 topleft = { x, y };

  higanbana::vector<float4> vertices;
  float left = topleft.x * 2.f - 1.f;
  float top = 1.f - (topleft.y * 2.f);
  float right = left + size.x*2.f; // -0.8f + 1.f
  float bottom = top - size.y*2.f; // 0.8f - 1.f

  vertices.push_back(float4{ left, bottom,  0.f, 0.f });
  vertices.push_back(float4{ left,  top,    0.f, 1.f });
  vertices.push_back(float4{ right, top,    1.f, 1.f });

  vertices.push_back(float4{ right, bottom, 1.f, 0.f });
  vertices.push_back(float4{ left,  bottom, 0.f, 0.f });
  vertices.push_back(float4{ right, top,    1.f, 1.f });

  auto verts = device.dynamicBuffer<float4>(vertices, higanbana::FormatType::Float32RGBA);

  auto args = device.createShaderArguments(ShaderArgumentsDescriptor("Opaque Arguments", m_input)
    .bind("vertices", verts)
    .bind("source", source));
  auto binding = node.bind(pipeline);
  binding.arguments(0, args);
  node.draw(binding, 6, 1);
}

void Blitter::blitImage(higanbana::GpuGroup& device, higanbana::CommandGraphNode& node, higanbana::TextureSRV& source, FitMode mode)
{
  using namespace higanbana;
  float dstScale = float(target.desc().desc.width) / float(target.desc().desc.height);
  float srcScale = float(source.desc().desc.width) / float(source.desc().desc.height);
  //float scaleDiff = dstScale - srcScale;

  float x = 2.f;
  float y = 2.f;

  float offsetX = 0.f;
  float offsetY = 0.f;
  if (mode == FitMode::Fit)
  {
    if (dstScale > srcScale)
    {
      x *= 1.f - (dstScale - srcScale) / dstScale;
      offsetX = (2.f - x) / 2.f;
    }
    else
    {
      y *= 1.f - (srcScale - dstScale) / srcScale;
      offsetY = (2.f - y) / 2.f;
    }
  }
  else if (mode == FitMode::Fill)
  {
    if (dstScale < srcScale)
    {
      x *= 1.f - (dstScale - srcScale) / dstScale;
      offsetX = (2.f - x) / 2.f;
    }
    else
    {
      y *= 1.f - (srcScale - dstScale) / srcScale;
      offsetY = (2.f - y) / 2.f;
    }
  }

  higanbana::vector<float4> vertices;
  float left = offsetX - 1.f;
  float top = 1.f - offsetY;
  float right = left + x; // -0.8f + 1.f
  float bottom = top - y; // 0.8f - 1.f

  vertices.push_back(float4{ left, bottom,  0.f, 0.f });
  vertices.push_back(float4{ left,  top,    0.f, 1.f });
  vertices.push_back(float4{ right, top,    1.f, 1.f });

  vertices.push_back(float4{ right, bottom, 1.f, 0.f });
  vertices.push_back(float4{ left,  bottom, 0.f, 0.f });
  vertices.push_back(float4{ right, top,    1.f, 1.f });

  auto verts = device.dynamicBuffer<float4>(vertices, higanbana::FormatType::Float32RGBA);
  auto args = device.createShaderArguments(ShaderArgumentsDescriptor("Opaque Arguments", m_input)
    .bind("vertices", verts)
    .bind("source", source));
  auto binding = node.bind(pipeline);
  binding.arguments(0, args);
  node.draw(binding, 6, 1);
}
}
}