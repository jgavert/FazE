#pragma once

#include "graphics/common/backend.hpp"

#include "graphics/common/heap_descriptor.hpp"
#include "graphics/common/pipeline.hpp"
#include "core/filesystem/filesystem.hpp"
#include "core/system/PageAllocator.hpp"
#include "core/datastructures/proxy.hpp"
#include "graphics/common/intermediatelist.hpp"

#include <string>
#include <atomic>

namespace faze
{
  enum class GraphicsApi
  {
    Vulkan,
    DX12,
	All
  };

  const char* toString(GraphicsApi api);

  enum class DeviceType
  {
    Unknown,
    IntegratedGpu,
    DiscreteGpu,
    VirtualGpu,
    Cpu
  };

  enum class VendorID
  {
    Amd, // = 4098,
    Nvidia, // = 4318,
    Intel, // = 32902
    Unknown
  };

  struct GpuInfo
  {
    int id;
    std::string name;
    int64_t memory;
    VendorID vendor;
    DeviceType type;
    bool canPresent;
    GraphicsApi api;
    uint32_t apiVersion;
    std::string apiVersionStr;
  };

  struct MemoryRequirements
  {
    size_t alignment;
    size_t bytes;
    int64_t heapType;
  };

  class Buffer;
  class SharedBuffer;
  class Texture;
  class SharedTexture;

  class BufferSRV;
  class BufferUAV;
  class TextureSRV;
  class TextureUAV;
  class TextureRTV;
  class TextureDSV;
  class DynamicBufferView;

  class Swapchain;
  class Renderpass;
  class GraphicsSurface;
  class Window;
  class CommandGraph;
  class CpuImage;

  class GpuSemaphore;
  class GpuSharedSemaphore;

  namespace backend
  {
    class FenceImpl;
    class SemaphoreImpl;
    class CommandBufferImpl;
    namespace prototypes
    {
      class DeviceImpl;
      class SubsystemImpl;
      class HeapImpl;
      class BufferImpl;
      class TextureImpl;
      class BufferViewImpl;
      class TextureViewImpl;
      class SwapchainImpl;
      class GraphicsSurfaceImpl;
    }

#if defined(FAZE_PLATFORM_WINDOWS)
    struct SharedHandle
    {
      void* handle;
    };
#else
    struct SharedHandle
    {
      int woot;
    };
#endif

    struct GpuHeap
    {
      std::shared_ptr<prototypes::HeapImpl> impl;
      std::shared_ptr<HeapDescriptor> desc;

      GpuHeap(std::shared_ptr<prototypes::HeapImpl> impl, HeapDescriptor desc)
        : impl(impl)
        , desc(std::make_shared<HeapDescriptor>(desc))
      {
      }
    };

    struct HeapAllocation
    {
      GpuHeapAllocation allocation;
      GpuHeap heap;
    };

    class HeapManager
    {
      struct HeapBlock
      {
        uint64_t index;
        FixedSizeAllocator allocator;
        GpuHeap heap;
      };

      struct HeapVector
      {
        int alignment;
        int64_t type;
        vector<HeapBlock> heaps;
      };

      vector<HeapVector> m_heaps;
      const int64_t m_minimumHeapSize = 16 * 1024 * 1024;
      uint64_t m_heapIndex = 0;

      uint64_t m_totalMemory = 0;
    public:

      HeapAllocation allocate(prototypes::DeviceImpl* device, MemoryRequirements requirements);
      void release(GpuHeapAllocation allocation);

      vector<GpuHeap> emptyHeaps();
    };

    struct LiveCommandBuffer
    {
      vector<std::shared_ptr<backend::SemaphoreImpl>> wait;
      vector<std::shared_ptr<backend::CommandBufferImpl>> lists;
      vector<std::shared_ptr<backend::SemaphoreImpl>> signal;
      std::shared_ptr<backend::FenceImpl> fence;
      std::shared_ptr<vector<IntermediateList>> intermediateLists;
    };

    struct DeviceData : std::enable_shared_from_this<DeviceData>
    {
      std::shared_ptr<prototypes::DeviceImpl> m_impl;
      HeapManager m_heaps;

      std::shared_ptr<ResourceTracker<prototypes::BufferImpl>> m_trackerbuffers;
      std::shared_ptr<ResourceTracker<prototypes::TextureImpl>> m_trackertextures;

      std::shared_ptr<std::atomic<int64_t>> m_idGenerator;
      deque<LiveCommandBuffer> m_buffers;

      int64_t newId() { return (*m_idGenerator)++; }

      DeviceData(std::shared_ptr<prototypes::DeviceImpl> impl);
      DeviceData(DeviceData&& data) = default;
      DeviceData(const DeviceData& data) = delete;
      DeviceData& operator=(DeviceData&& data) = default;
      DeviceData& operator=(const DeviceData& data) = delete;
      ~DeviceData();

      void checkCompletedLists();
      void gc();
      void garbageCollection();
      void waitGpuIdle();
      Swapchain createSwapchain(GraphicsSurface& surface, SwapchainDescriptor descriptor);
      void adjustSwapchain(Swapchain& swapchain, SwapchainDescriptor descriptor);
      TextureRTV acquirePresentableImage(Swapchain& swapchain);
      TextureRTV* tryAcquirePresentableImage(Swapchain& swapchain);

      Renderpass createRenderpass();
      ComputePipeline createComputePipeline(ComputePipelineDescriptor desc);
      GraphicsPipeline createGraphicsPipeline(GraphicsPipelineDescriptor desc);

      Buffer createBuffer(ResourceDescriptor desc);
      SharedBuffer createSharedBuffer(DeviceData& secondary, ResourceDescriptor desc);
      Texture createTexture(ResourceDescriptor desc);
      SharedTexture createSharedTexture(DeviceData& secondary, ResourceDescriptor desc);

      BufferSRV createBufferSRV(Buffer texture, ShaderViewDescriptor viewDesc);
      BufferUAV createBufferUAV(Buffer texture, ShaderViewDescriptor viewDesc);
      TextureSRV createTextureSRV(Texture texture, ShaderViewDescriptor viewDesc);
      TextureUAV createTextureUAV(Texture texture, ShaderViewDescriptor viewDesc);
      TextureRTV createTextureRTV(Texture texture, ShaderViewDescriptor viewDesc);
      TextureDSV createTextureDSV(Texture texture, ShaderViewDescriptor viewDesc);

      GpuSemaphore createSemaphore();
      GpuSharedSemaphore createSharedSemaphore(DeviceData& secondary);

      DynamicBufferView dynamicBuffer(MemView<uint8_t> view, FormatType type);
      DynamicBufferView dynamicBuffer(MemView<uint8_t> view, unsigned stride);
      // streaming

      bool uploadInitialTexture(Texture& tex, CpuImage& image);

      // commandgraph
      void submit(Swapchain& swapchain, CommandGraph graph);
      void submit(CommandGraph graph);
      void explicitSubmit(CommandGraph graph);
      void present(Swapchain& swapchain);
    };

    struct SubsystemData : std::enable_shared_from_this<SubsystemData>
    {
      std::shared_ptr<prototypes::SubsystemImpl> implDX12;
      std::shared_ptr<prototypes::SubsystemImpl> implVulkan;
      const char* appName;
      unsigned appVersion;
      const char* engineName;
      unsigned engineVersion;

      SubsystemData(const char* appName, unsigned appVersion = 1, const char* engineName = "faze", unsigned engineVersion = 1);
      vector<GpuInfo> availableGpus(GraphicsApi api = GraphicsApi::All);
      GpuDevice createDevice(FileSystem& fs, GpuInfo gpu);
      GraphicsSurface createSurface(Window& window, GpuInfo gpu);
    };
  }
}