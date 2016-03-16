#pragma once
#include "VulkanQueue.hpp"
#include "VulkanCmdBuffer.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanTexture.hpp"
#include "VulkanHeap.hpp"
#include "VulkanPipeline.hpp"
#include "core/src/memory/ManagedResource.hpp"
#include "gfxvk/src/Graphics/ResourceDescriptor.hpp"
#include "gfxvk/src/Graphics/PipelineDescriptor.hpp"
#include <vulkan/vk_cpp.h>

class VulkanGpuDevice
{
private:


  vk::AllocationCallbacks m_alloc_info;
  FazPtrVk<vk::Device>    m_device;
  bool                    m_debugLayer;
  std::vector<vk::QueueFamilyProperties> m_queues;
  bool                    m_singleQueue;
  bool                    m_onlySeparateQueues;
  FazPtrVk<vk::Queue>     m_internalUniversalQueue;
  bool                    m_uma;
  struct FreeQueues
  {
    int universalIndex;
    int graphicsIndex;
    int computeIndex;
    int dmaIndex;
    std::vector<uint32_t> universal;
    std::vector<uint32_t> graphics;
    std::vector<uint32_t> compute;
    std::vector<uint32_t> dma;
  } m_freeQueueIndexes;

  struct MemoryTypes
  {
    int deviceLocalIndex;
    int hostNormalIndex;
    int hostCachedIndex; // probably not needed
    int deviceHostIndex; // default for uma, when discrete has this... what
  } m_memoryTypes;

public:
  VulkanGpuDevice(
    FazPtrVk<vk::Device> device,
    vk::AllocationCallbacks alloc_info,
    std::vector<vk::QueueFamilyProperties> queues,
    vk::PhysicalDeviceMemoryProperties memProp,
    bool debugLayer);
  bool isValid();
  VulkanQueue createDMAQueue();
  VulkanQueue createComputeQueue();
  VulkanQueue createGraphicsQueue();
  VulkanCmdBuffer createDMACommandBuffer();
  VulkanCmdBuffer createComputeCommandBuffer();
  VulkanCmdBuffer createGraphicsCommandBuffer();
  VulkanPipeline createGraphicsPipeline(GraphicsPipelineDescriptor desc);
  VulkanPipeline createComputePipeline(ComputePipelineDescriptor desc);
  VulkanMemoryHeap createMemoryHeap(HeapDescriptor desc);
  VulkanBuffer createBuffer(ResourceDescriptor desc);
  VulkanTexture createTexture(ResourceDescriptor desc);
  // shader views
  VulkanBufferShaderView createBufferView(VulkanBuffer targetTexture, ShaderViewDescriptor viewDesc = ShaderViewDescriptor());
  VulkanTextureShaderView createTextureView(VulkanTexture targetTexture, ShaderViewDescriptor viewDesc = ShaderViewDescriptor());

};

using GpuDeviceImpl = VulkanGpuDevice;