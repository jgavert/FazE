#pragma once
#include "higanbana/graphics/dx12/dx12resources.hpp"
#if defined(HIGANBANA_PLATFORM_WINDOWS)
#include "higanbana/graphics/common/command_packets.hpp"

namespace higanbana
{
  namespace backend
  {
    class DX12CommandList : public CommandBufferImpl
    {
      QueueType m_type;
      std::shared_ptr<DX12CommandBuffer> m_buffer;
      std::shared_ptr<DX12UploadHeap> m_constants;
      std::shared_ptr<DX12ReadbackHeap> m_readback;
      std::shared_ptr<DX12QueryHeap> m_queryheap;
      std::shared_ptr<DX12DynamicDescriptorHeap> m_descriptors;
      DX12CPUDescriptor m_nullBufferUAV;
      DX12CPUDescriptor m_nullBufferSRV;
      vector<DX12Query> queries;
      ReadbackBlock readback;

      UploadLinearAllocator m_constantsAllocator;
      LinearDescriptorAllocator m_descriptorAllocator;
      DX12GPUDescriptor m_shaderDebugTable;

      ResourceHandle  m_boundGfxSets[HIGANBANA_USABLE_SHADER_ARGUMENT_SETS] = {};
      ResourceHandle  m_boundCompSets[HIGANBANA_USABLE_SHADER_ARGUMENT_SETS] = {};
      ViewResourceHandle m_boundIndexBufferHandle = {};
      D3D12_INDEX_BUFFER_VIEW m_ib = {};

      std::shared_ptr<FreeableResources> m_freeResources;

      UploadBlock allocateConstants(size_t size);
      DynamicDescriptorBlock allocateDescriptors(size_t size);
      void handleBindings(DX12Device* dev, D3D12GraphicsCommandList*, gfxpacket::ResourceBinding& binding);
      //void addDepedencyDataAndSolve(DX12DependencySolver* solver, backend::IntermediateList& list);
      void addCommands(DX12Device* device, D3D12GraphicsCommandList* buffer, MemView<backend::CommandBuffer>& buffers, BarrierSolver& solver);

    public:
      DX12CommandList(
        QueueType type,
        std::shared_ptr<DX12CommandBuffer> buffer,
        std::shared_ptr<DX12UploadHeap> constants,
        std::shared_ptr<DX12ReadbackHeap> readback,
        std::shared_ptr<DX12QueryHeap> queryheap,
        std::shared_ptr<DX12DynamicDescriptorHeap> descriptors,
        DX12CPUDescriptor nullBufferUAV,
        DX12CPUDescriptor nullBufferSRV,
        DX12GPUDescriptor shaderDebugTable);

      void fillWith(std::shared_ptr<prototypes::DeviceImpl>, MemView<backend::CommandBuffer>& buffers, BarrierSolver& solver) override;
      void readbackTimestamps(std::shared_ptr<prototypes::DeviceImpl>, vector<GraphNodeTiming>& nodes) override;

      bool closed() const
      {
        return m_buffer->closed();
      }

      ID3D12GraphicsCommandList* list()
      {
        return m_buffer->list();
      }
    };
  }
}
#endif