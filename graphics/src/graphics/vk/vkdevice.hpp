#pragma once
#include <graphics/vk/vkresources.hpp>
namespace faze
{
  namespace backend
  {
    class VulkanDevice : public prototypes::DeviceImpl
    {
    private:
      vk::Device                  m_device;
      vk::PhysicalDevice		      m_physDevice;
      bool                        m_debugLayer;
      std::vector<vk::QueueFamilyProperties> m_queues;
      bool                        m_singleQueue;
      bool                        m_computeQueues;
      bool                        m_dmaQueues;
      bool                        m_graphicQueues;
      GpuInfo                     m_info;
      ShaderStorage               m_shaders;
      int64_t                     m_resourceID = 1;

      int                         m_mainQueueIndex;
      int                         m_computeQueueIndex = -1;
      int                         m_copyQueueIndex = -1;
      vk::Queue                   m_mainQueue;
      vk::Queue                   m_computeQueue;
      vk::Queue                   m_copyQueue;

      vk::Sampler                 m_bilinearSampler;
      vk::Sampler                 m_pointSampler;
      vk::Sampler                 m_bilinearSamplerWrap;
      vk::Sampler                 m_pointSamplerWrap;

      Rabbitpool2<VulkanSemaphore>    m_semaphores;
      Rabbitpool2<VulkanFence>        m_fences;
      Rabbitpool2<VulkanCommandList>  m_copyListPool;
      Rabbitpool2<VulkanCommandList>  m_computeListPool;
      Rabbitpool2<VulkanCommandList>  m_graphicsListPool;

      unordered_map<size_t, std::shared_ptr<vk::RenderPass>> m_renderpasses;

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

      // new stuff

      std::shared_ptr<SequenceTracker> m_seqTracker;
      std::shared_ptr<VulkanUploadHeap> m_dynamicUpload;

      struct Garbage
      {
        vector<VkUploadBlock> dynamicBuffers;
        vector<vk::Image> textures;
        vector<vk::Buffer> buffers;
        vector<vk::ImageView> textureviews;
        vector<vk::BufferView> bufferviews;
        vector<vk::Pipeline> pipelines;
        vector<vk::DescriptorSetLayout> descriptorSetLayouts;
        vector<vk::PipelineLayout>      pipelineLayouts;
        vector<vk::DeviceMemory> heaps;
      };

      std::shared_ptr<Garbage> m_trash;
      deque<std::pair<SeqNum, Garbage>> m_collectableTrash;

      // new new stuff
      // HandleVector<VulkanTexture>
    public:
      VulkanDevice(
        vk::Device device,
        vk::PhysicalDevice physDev,
        FileSystem& fs,
        std::vector<vk::QueueFamilyProperties> queues,
        GpuInfo info,
        bool debugLayer);
      ~VulkanDevice();

      vk::Device native() { return m_device; }

      std::shared_ptr<vk::RenderPass> createRenderpass(const vk::RenderPassCreateInfo& info);

      void updatePipeline(GraphicsPipeline& pipeline, vk::RenderPass rp, gfxpacket::RenderpassBegin& subpass);
      void updatePipeline(ComputePipeline& pipeline);

      // implementation
      std::shared_ptr<prototypes::SwapchainImpl> createSwapchain(GraphicsSurface& surface, SwapchainDescriptor descriptor) override;
      void adjustSwapchain(std::shared_ptr<prototypes::SwapchainImpl> sc, SwapchainDescriptor descriptor) override;
      vector<std::shared_ptr<prototypes::TextureImpl>> getSwapchainTextures(std::shared_ptr<prototypes::SwapchainImpl> sc, HandleManager& handles) override;
      int tryAcquirePresentableImage(std::shared_ptr<prototypes::SwapchainImpl> swapchain) override;
      int acquirePresentableImage(std::shared_ptr<prototypes::SwapchainImpl> swapchain) override;

      void collectTrash() override;
      void waitGpuIdle() override;
      MemoryRequirements getReqs(ResourceDescriptor desc) override;

      std::shared_ptr<prototypes::RenderpassImpl> createRenderpass() override;
      vector<vk::DescriptorSetLayoutBinding> gatherSetLayoutBindings(ShaderInputDescriptor desc, vk::ShaderStageFlags flags);
      std::shared_ptr<prototypes::PipelineImpl> createPipeline(GraphicsPipelineDescriptor desc) override;
      std::shared_ptr<prototypes::PipelineImpl> createPipeline(ComputePipelineDescriptor desc) override;

      GpuHeap createHeap(HeapDescriptor desc) override;

      std::shared_ptr<prototypes::BufferImpl> createBuffer(ResourceDescriptor& desc) override;
      std::shared_ptr<prototypes::BufferImpl> createBuffer(HeapAllocation allocation, ResourceDescriptor& desc) override;
      std::shared_ptr<prototypes::BufferViewImpl> createBufferView(std::shared_ptr<prototypes::BufferImpl> buffer, ResourceDescriptor& desc, ShaderViewDescriptor& viewDesc) override;
      std::shared_ptr<prototypes::TextureImpl> createTexture(ResourceDescriptor& desc) override;
      std::shared_ptr<prototypes::TextureImpl> createTexture(HeapAllocation allocation, ResourceDescriptor& desc) override;
      std::shared_ptr<prototypes::TextureViewImpl> createTextureView(std::shared_ptr<prototypes::TextureImpl> buffer, ResourceDescriptor& desc, ShaderViewDescriptor& viewDesc) override;

      std::shared_ptr<SemaphoreImpl> createSharedSemaphore() override { return nullptr; };

      std::shared_ptr<SharedHandle> openSharedHandle(std::shared_ptr<SemaphoreImpl>) override { return nullptr; };
      std::shared_ptr<SharedHandle> openSharedHandle(HeapAllocation) override { return nullptr; };
      std::shared_ptr<SharedHandle> openSharedHandle(std::shared_ptr<prototypes::TextureImpl>) override { return nullptr; };
      std::shared_ptr<SemaphoreImpl> createSemaphoreFromHandle(std::shared_ptr<SharedHandle>) override { return nullptr; };
      std::shared_ptr<prototypes::BufferImpl> createBufferFromHandle(std::shared_ptr<SharedHandle>, HeapAllocation, ResourceDescriptor&) override { return nullptr; };
      std::shared_ptr<prototypes::TextureImpl> createTextureFromHandle(std::shared_ptr<SharedHandle>, ResourceDescriptor&) override { return nullptr; };

      std::shared_ptr<prototypes::DynamicBufferViewImpl> dynamic(MemView<uint8_t> bytes, FormatType format) override;
      std::shared_ptr<prototypes::DynamicBufferViewImpl> dynamic(MemView<uint8_t> bytes, unsigned stride) override;
      std::shared_ptr<prototypes::DynamicBufferViewImpl> dynamicImage(MemView<uint8_t> bytes, unsigned rowPitch) override;

      // commandlist stuff
      VulkanCommandList createCommandBuffer(int queueIndex);
      void resetListNative(VulkanCommandList list);
      std::shared_ptr<CommandBufferImpl> createDMAList() override;
      std::shared_ptr<CommandBufferImpl> createComputeList() override;
      std::shared_ptr<CommandBufferImpl> createGraphicsList() override;
      std::shared_ptr<SemaphoreImpl>     createSemaphore() override;
      std::shared_ptr<FenceImpl>         createFence() override;

      void submitToQueue(
        vk::Queue queue,
        MemView<std::shared_ptr<CommandBufferImpl>> lists,
        MemView<std::shared_ptr<SemaphoreImpl>>     wait,
        MemView<std::shared_ptr<SemaphoreImpl>>     signal,
        MemView<std::shared_ptr<FenceImpl>>         fence);

      void submitDMA(
        MemView<std::shared_ptr<CommandBufferImpl>> lists,
        MemView<std::shared_ptr<SemaphoreImpl>>     wait,
        MemView<std::shared_ptr<SemaphoreImpl>>     signal,
        MemView<std::shared_ptr<FenceImpl>>         fence) override;

      void submitCompute(
        MemView<std::shared_ptr<CommandBufferImpl>> lists,
        MemView<std::shared_ptr<SemaphoreImpl>>     wait,
        MemView<std::shared_ptr<SemaphoreImpl>>     signal,
        MemView<std::shared_ptr<FenceImpl>>         fence) override;

      void submitGraphics(
        MemView<std::shared_ptr<CommandBufferImpl>> lists,
        MemView<std::shared_ptr<SemaphoreImpl>>     wait,
        MemView<std::shared_ptr<SemaphoreImpl>>     signal,
        MemView<std::shared_ptr<FenceImpl>>         fence) override;

      void waitFence(std::shared_ptr<FenceImpl>     fence) override;
      bool checkFence(std::shared_ptr<FenceImpl>    fence) override;

      void present(std::shared_ptr<prototypes::SwapchainImpl> swapchain, std::shared_ptr<SemaphoreImpl> renderingFinished) override;
    };
  }
}