#pragma once

#include "resources.hpp"
#include "resource_descriptor.hpp"
#include "buffer.hpp"
#include "texture.hpp"

namespace faze
{
  class GpuDevice : private backend::SharedState<backend::DeviceData>
  {
  public:
    GpuDevice() = default;
    GpuDevice(backend::DeviceData data)
    {
      makeState(data);
    }
    GpuDevice(StatePtr state)
    {
      m_state = std::move(state);
    }

    StatePtr state()
    {
      return m_state;
    }
    Buffer createBuffer(ResourceDescriptor descriptor)
    {
      auto buf = S().createBuffer(descriptor);
      buf.setParent(this);
      return buf;
    }

    Texture createTexture(ResourceDescriptor descriptor)
    {
      auto tex = S().createTexture(descriptor);
      tex.setParent(this);
      return tex;
    }
  };
};