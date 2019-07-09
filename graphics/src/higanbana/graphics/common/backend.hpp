#pragma once

#include "higanbana/graphics/common/resource_descriptor.hpp"
#include <higanbana/core/system/PageAllocator.hpp>
#include <memory>
#include <vector>
#include <mutex>

#include <higanbana/core/global_debug.hpp>

namespace higanbana
{
  class GpuGroup;

  struct GpuHeapAllocation
  {
    uint64_t index;
    int alignment;
    int64_t heapType;
    PageBlock block;

    bool valid() { return alignment != -1 && index != static_cast<uint64_t>(-1); }
  };

  namespace backend
  {
    struct DeviceGroupData;

    template <typename State>
    class SharedState
    {
    protected:
      using StatePtr = std::shared_ptr<State>;

      StatePtr m_state;
    public:

      template <typename... Ts>
      void makeState(Ts&&... ts)
      {
        m_state = std::make_shared<State>(std::forward<Ts>(ts)...);
      }

      const State& S() const { return *m_state; }
      State& S() { return *m_state; }

      bool valid() const { return !!m_state; }
    };

    constexpr size_t createMaskWithNBitsSet(size_t count)
    {
      return (count == 0ll) ? 0ll : (1ll << (count - 1ll)) | createMaskWithNBitsSet(count - 1ll);
    }

    struct RawView
    {
      size_t view;

      /*
      template <typename T>
      void ptr(const T* val)
      {
        size_t storable = reinterpret_cast<size_t>(val);
        auto checkFirstBits = storable & createMaskWithNBitsSet(4);
        HIGAN_ASSERT(checkFirstBits == 0, "Expecting first 4 bits to be empty.");
        view &= createMaskWithNBitsSet(4);
        view |= storable
      }

      template <typename T>
      void val(const T val)
      {
        size_t storable = static_cast<size_t>(val);
        auto checkFirstBits = storable & createMaskWithNBitsSet(4);
        HIGAN_ASSERT(checkFirstBits == 0, "Expecting first 4 bits to be empty.");
        view &= createMaskWithNBitsSet(4);
        view |= storable;
      }

      void storeSmallValue(unsigned val)
      {
        val &= createMaskWithNBitsSet(4);
        view &= ~(createMaskWithNBitsSet(4));
        view |= val;
      }*/
    };

    struct TrackedState
    {
      size_t resPtr;
      size_t statePtr;
      size_t additionalInfo;
      bool readonly;

      static constexpr size_t arrayBits = 11;
      static constexpr size_t mipBits = 4;

      // layout is arraySize(11), arrayBase(11), mipSize(4), mipBase(4), rest of additionalinfo.
      void storeSubresourceRange(unsigned mipBase, unsigned mipSize, unsigned arrayBase, unsigned arraySize)
      {
        size_t res = mipBase;
        res = res << mipBits;
        res |= mipSize;
        res = res << arrayBits;
        res |= arrayBase;
        res = res << arrayBits;
        res |= arraySize;
        additionalInfo &= ~(createMaskWithNBitsSet(arrayBits + arrayBits + mipBits + mipBits));
        additionalInfo |= res;
      }

      void storeTotalMipLevels(unsigned mips)
      {
        size_t res = mips;
        res = res << (arrayBits + arrayBits + mipBits + mipBits);
        size_t mask = createMaskWithNBitsSet(mipBits) << (arrayBits + arrayBits + mipBits + mipBits);

        additionalInfo &= ~(mask);
        additionalInfo |= res;
      }

      unsigned mip() const
      {
        return static_cast<unsigned>((additionalInfo >> (arrayBits + arrayBits + mipBits)) & createMaskWithNBitsSet(mipBits));
      }

      unsigned slice() const
      {
        return static_cast<unsigned>((additionalInfo >> arrayBits) & createMaskWithNBitsSet(arrayBits));
      }

      unsigned arraySize() const
      {
        auto size = static_cast<unsigned>(additionalInfo & createMaskWithNBitsSet(arrayBits));
        return (size == 0) ? 2048 : size;
      }

      unsigned mipLevels() const
      {
        auto size = static_cast<unsigned>((additionalInfo >> (arrayBits + arrayBits)) & createMaskWithNBitsSet(mipBits));
        return (size == 0) ? 16 : size;
      }

      unsigned totalMipLevels() const
      {
        return static_cast<unsigned>((additionalInfo >> (arrayBits + arrayBits + mipBits + mipBits)) & createMaskWithNBitsSet(mipBits));
      }

      bool operator==(const TrackedState& a) const noexcept
      {
        return resPtr == a.resPtr;
      }
    };

/*
    template <typename TrackedResource>
    class ResourceTracker : public std::enable_shared_from_this<ResourceTracker<TrackedResource>>
    {
      std::vector<std::shared_ptr<TrackedResource>> m_disposableObjects;
      std::vector<GpuHeapAllocation>                m_disposableMemory;
      std::mutex                                    m_lock;
    public:
      ResourceTracker()
      {
      }

      std::shared_ptr<int64_t> makeTracker(int64_t id, GpuHeapAllocation allocation)
      {
        auto dest = this->shared_from_this();
        return std::shared_ptr<int64_t>(new int64_t(id), [allocation, dest](int64_t* t)
        {
          std::lock_guard<std::mutex> lock(dest->m_lock);
          dest->m_disposableMemory.emplace_back(allocation);
          delete t;
        });
      }

      std::vector<std::shared_ptr<TrackedResource>> getResources()
      {
        std::lock_guard<std::mutex> lock(m_lock);
        auto vec = m_disposableObjects;
        m_disposableObjects.clear();
        return vec;
      }

      std::vector<GpuHeapAllocation> getAllocations()
      {
        std::lock_guard<std::mutex> lock(m_lock);
        auto vec = m_disposableMemory;
        m_disposableMemory.clear();
        return vec;
      }
    };
    */

    class GpuGroupChild
    {
      std::weak_ptr<DeviceGroupData> m_parentDevice;
    public:
      GpuGroupChild() = default;
      GpuGroupChild(std::weak_ptr<DeviceGroupData> device)
        : m_parentDevice(device)
      {}

      void setParent(GpuGroup *device);
      GpuGroup device();
    };
  }
}