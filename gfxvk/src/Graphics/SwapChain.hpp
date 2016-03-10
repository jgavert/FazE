#pragma once
#include "Texture.hpp"

#include <vector>

class SwapChain
{
private:
  friend class GpuDevice;

  void* m_SwapChain;
  std::vector<TextureRTV> m_resources;

  SwapChain(void* SwapChain)
    : m_SwapChain(std::move(SwapChain))
  {}

  SwapChain(void* SwapChain, std::vector<TextureRTV> resources)
    : m_SwapChain(std::move(SwapChain))
    , m_resources(resources)
  {
  }

  SwapChain():m_SwapChain(nullptr)
  {
  }
public:

  ~SwapChain()
  {
  }

  TextureRTV& operator[](size_t )
  {
    return m_resources.at(0);
  }

  size_t rtvCount()
  {
    return m_resources.size();
  }

  unsigned GetCurrentBackBufferIndex()
  {
    return 0;
  }

  void present(unsigned, unsigned)
  {

  }

  bool valid()
  {
    return true;
  }

  void* operator->() const throw()
  {
    return m_SwapChain;
  }

};
