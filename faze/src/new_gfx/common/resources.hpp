#pragma once

#include "backend.hpp"

#include "heap_descriptor.hpp"
#include "core/src/filesystem/filesystem.hpp"
#include "core/src/system/PageAllocator.hpp"
#include "core/src/datastructures/proxy.hpp"
#include <string>
#include <atomic>

namespace faze
{
  enum class GraphicsApi
  {
    Vulkan,
    DX12
  };

  enum class DeviceType
  {
    Unknown,
    IntegratedGpu,
    DiscreteGpu,
    VirtualGpu,
    Cpu
  };

  struct GpuInfo
  {
    int id;
    std::string name;
    int64_t memory;
    int vendor;
    DeviceType type;
    bool canPresent;
  };

  struct MemoryRequirements
  {
    size_t alignment;
    size_t bytes;
    HeapType type;
  };

  struct GpuHeapAllocation
  {
    int index;
    int alignment;
    HeapType type;
    PageBlock block;

    bool valid() { return alignment != -1 && index != -1; }
  };

  class Buffer;
  class Texture;

  namespace backend
  {
    namespace prototypes
    {
      class DeviceImpl;
      class SubsystemImpl;
      class HeapImpl;
      class BufferImpl;
      class TextureImpl;
    }

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
        int index;
        PageAllocator allocator;
        GpuHeap heap;
      };

      struct HeapVector
      {
        int alignment;
        HeapType type;
        vector<HeapBlock> heaps;
      };

      vector<HeapVector> m_heaps;
      const int64_t m_minimumHeapSize = 32 * 1024 * 1024;
    public:

      HeapAllocation allocate(prototypes::DeviceImpl* device, MemoryRequirements requirements);
      void release(GpuHeapAllocation allocation);

      vector<GpuHeap> emptyHeaps();
    };

    struct DeviceData : std::enable_shared_from_this<DeviceData>
    {
      std::shared_ptr<prototypes::DeviceImpl> m_impl;
      HeapManager m_heaps;

      std::shared_ptr<ResourceTracker<prototypes::BufferImpl>> m_bufferTracker;
      std::shared_ptr<ResourceTracker<prototypes::TextureImpl>> m_textureTracker;

      std::shared_ptr<std::atomic<int64_t>> m_idGenerator;

      int64_t newId() { return (*m_idGenerator)++; }

      DeviceData(std::shared_ptr<prototypes::DeviceImpl> impl);
      ~DeviceData();
      void waitGpuIdle();
      Buffer createBuffer(ResourceDescriptor desc);
      void createBufferView(ShaderViewDescriptor desc);
      Texture createTexture(ResourceDescriptor desc);
    };

    struct SubsystemData : std::enable_shared_from_this<SubsystemData>
    {
      std::shared_ptr<prototypes::SubsystemImpl> impl;
      const char* appName;
      unsigned appVersion;
      const char* engineName;
      unsigned engineVersion;

      SubsystemData(GraphicsApi api, const char* appName, unsigned appVersion = 1, const char* engineName = "faze", unsigned engineVersion = 1);
      std::string gfxApi();
      vector<GpuInfo> availableGpus();
      GpuDevice createDevice(FileSystem& fs, GpuInfo gpu);
    };
  }
}