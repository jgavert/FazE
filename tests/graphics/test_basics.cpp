#include "tests/graphics/graphics_config.hpp"
#include <catch2/catch.hpp>

SHADER_STRUCT(TestConsts,
  float value;
);

TEST_CASE_METHOD(GraphicsFixture, "basic compute and readback") {
  float dynamicBufferInput = 2.f;
  float constantInput = 8.f;

  auto inputArgumentsLayout = gpu().createShaderArgumentsLayout(ShaderArgumentsLayoutDescriptor()
    .readOnly(ShaderResourceType::Buffer, "float", "input"));
  auto outputArgumentsLayout = gpu().createShaderArgumentsLayout(ShaderArgumentsLayoutDescriptor()
    .readWrite(ShaderResourceType::Buffer, "float", "output"));

  auto comp = gpu().createComputePipeline(ComputePipelineDescriptor()
    .setInterface(PipelineInterfaceDescriptor()
      .constants<TestConsts>()
      .shaderArguments(0, inputArgumentsLayout)
      .shaderArguments(1, outputArgumentsLayout))
    .setShader("addTogether")
    .setThreadGroups(uint3(1, 1, 1)));

  auto outputBuffer = gpu().createBufferUAV(ResourceDescriptor()
    .setFormat(FormatType::Float32)
    .setUsage(ResourceUsage::GpuRW)
    .setCount(1)); 

  auto outputArguments = gpu().createShaderArguments(ShaderArgumentsDescriptor("output arguments", inputArgumentsLayout)
    .bind("output", outputBuffer));

  auto graph = gpu().createGraph();

  auto upload = gpu().dynamicBuffer(MemView<float>(&dynamicBufferInput, 1), FormatType::Float32);

  auto inputArguments = gpu().createShaderArguments(ShaderArgumentsDescriptor("input arguments", inputArgumentsLayout)
    .bind("input", upload));

  auto node = graph.createPass("computeCopy", QueueType::Compute);
  auto binding = node.bind(comp);

  TestConsts consts{};
  consts.value = constantInput;
  binding.constants(consts);

  binding.arguments(0, inputArguments);
  binding.arguments(1, outputArguments);

  node.dispatch(binding, uint3(1, 1, 1));

  auto readbackNode = graph.createPass("readback", QueueType::Dma);
  auto asyncReadback = readbackNode.readback(outputBuffer.buffer());
  
  graph.addPass(std::move(node));
  graph.addPass(std::move(readbackNode));

  gpu().submit(graph);
  gpu().waitGpuIdle();

  {
    auto rb = asyncReadback.get();
    auto data = rb.view<float>();

    REQUIRE(data[0] == 16.f);
  }
}