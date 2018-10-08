#pragma once

#include "backend.hpp"
#include "resources.hpp"
#include "resource_descriptor.hpp"
#include "prototypes.hpp"

namespace faze
{
  class Texture
  {
    std::shared_ptr<backend::prototypes::TextureImpl> impl;
    std::shared_ptr<int64_t> m_id;
    std::shared_ptr<ResourceDescriptor> m_desc;
    backend::TrackedState m_dependency;
  public:
    Texture()
      : m_desc(std::make_shared<ResourceDescriptor>())
    {
    }

    Texture(const Texture&) = default;
    Texture(Texture&&) = default;
    Texture& operator=(const Texture&) = default;
    Texture& operator=(Texture&&) = default;

    ~Texture()
    {
    }

    Texture(std::shared_ptr<backend::prototypes::TextureImpl> impl, std::shared_ptr<int64_t> id, std::shared_ptr<ResourceDescriptor> desc)
      : impl(impl)
      , m_id(id)
      , m_desc(desc)
      , m_dependency(impl->dependency())
    {
      m_dependency.storeSubresourceRange(0, m_desc->desc.miplevels, 0, m_desc->desc.arraySize);
      m_dependency.storeTotalMipLevels(m_desc->desc.miplevels);
    }

    ResourceDescriptor& desc()
    {
      return *m_desc;
    }

    std::shared_ptr<backend::prototypes::TextureImpl> native()
    {
      return impl;
    }

    int64_t id() const
    {
      return *m_id;
    }

    backend::TrackedState dependency()
    {
      return m_dependency;
    }

    int3 size3D()
    {
      return int3(m_desc->desc.width, m_desc->desc.height, m_desc->desc.depth);
    }
  };

  class TextureView
  {
    Texture tex;
    std::shared_ptr<backend::prototypes::TextureViewImpl> impl;
    int64_t m_id;
    backend::RawView m_view;
    backend::TrackedState m_dependency;

    FormatType m_format;
    LoadOp m_loadOp = LoadOp::Load;
    StoreOp m_storeOp = StoreOp::Store;
  public:
    TextureView() = default;

    TextureView(Texture tex, std::shared_ptr<backend::prototypes::TextureViewImpl> impl, int64_t id, const SubresourceRange& range, FormatType type)
      : tex(tex)
      , impl(impl)
      , m_id(id)
      , m_view(impl->view())
      , m_dependency(tex.dependency())
      , m_format(type)
    {
      /*F_LOG("Storing data %u %u %u %u\n", static_cast<unsigned>(range.mipOffset),
        static_cast<unsigned>(range.mipLevels),
        static_cast<unsigned>(range.sliceOffset),
        static_cast<unsigned>(range.arraySize));
        */
      m_dependency.storeSubresourceRange(
        static_cast<unsigned>(range.mipOffset),
        static_cast<unsigned>(range.mipLevels),
        static_cast<unsigned>(range.sliceOffset),
        static_cast<unsigned>(range.arraySize));
      /*
      F_LOG("verify data %u %u %u %u\n",
        m_dependency.mip(),
        m_dependency.mipLevels(),
        m_dependency.slice(),
        m_dependency.arraySize());
        */
    }

    ResourceDescriptor& desc()
    {
      return tex.desc();
    }

    std::shared_ptr<backend::prototypes::TextureViewImpl> native()
    {
      return impl;
    }

    Texture& texture()
    {
      return tex;
    }

    int64_t id() const
    {
      return m_id;
    }

    void setOp(LoadOp op)
    {
      m_loadOp = op;
    }

    void setOp(StoreOp op)
    {
      m_storeOp = op;
    }

    LoadOp loadOp()
    {
      return m_loadOp;
    }

    StoreOp storeOp()
    {
      return m_storeOp;
    }

    backend::RawView view() const
    {
      return m_view;
    }
    backend::TrackedState dependency() const
    {
      return m_dependency;
    }

    FormatType format()
    {
      return m_format;
    }

  };

  class TextureSRV : public TextureView
  {
  public:

    TextureSRV() = default;
    TextureSRV(Texture tex, std::shared_ptr<backend::prototypes::TextureViewImpl> impl, int64_t id, const SubresourceRange& range, FormatType type)
      : TextureView(tex, impl, id, range, type)
    {
    }

    TextureSRV& op(LoadOp op)
    {
      setOp(op);
      return *this;
    }

    TextureSRV& op(StoreOp op)
    {
      setOp(op);
      return *this;
    }
  };

  class TextureUAV : public TextureView
  {
  public:
    TextureUAV() = default;
    TextureUAV(Texture tex, std::shared_ptr<backend::prototypes::TextureViewImpl> impl, int64_t id, const SubresourceRange& range, FormatType type)
      : TextureView(tex, impl, id, range, type)
    {
    }

    TextureUAV& op(LoadOp op)
    {
      setOp(op);
      return *this;
    }

    TextureUAV& op(StoreOp op)
    {
      setOp(op);
      return *this;
    }
  };

  class TextureRTV : public TextureView
  {
  public:
    TextureRTV() = default;
    TextureRTV(Texture tex, std::shared_ptr<backend::prototypes::TextureViewImpl> impl, int64_t id, const SubresourceRange& range, FormatType type)
      : TextureView(tex, impl, id, range, type)
    {
    }

    TextureRTV& op(LoadOp op)
    {
      setOp(op);
      return *this;
    }

    TextureRTV& op(StoreOp op)
    {
      setOp(op);
      return *this;
    }
  };

  class TextureDSV : public TextureView
  {
  public:
    TextureDSV() = default;
    TextureDSV(Texture tex, std::shared_ptr<backend::prototypes::TextureViewImpl> impl, int64_t id, const SubresourceRange& range, FormatType type)
      : TextureView(tex, impl, id, range, type)
    {
    }

    TextureDSV& op(LoadOp op)
    {
      setOp(op);
      return *this;
    }

    TextureDSV& op(StoreOp op)
    {
      setOp(op);
      return *this;
    }
  };

  class SharedTexture
  {
    std::shared_ptr<backend::prototypes::TextureImpl> primaryImpl;
    std::shared_ptr<backend::prototypes::TextureImpl> secondaryImpl;
    std::shared_ptr<int64_t> id;
    std::shared_ptr<ResourceDescriptor> m_desc;
  public:
    SharedTexture()
      : m_desc(std::make_shared<ResourceDescriptor>())
    {
    }
    SharedTexture(std::shared_ptr<backend::prototypes::TextureImpl> primaryImpl, std::shared_ptr<backend::prototypes::TextureImpl> secondaryImpl, std::shared_ptr<int64_t> id, std::shared_ptr<ResourceDescriptor> desc)
      : primaryImpl(primaryImpl)
      , secondaryImpl(secondaryImpl)
      , id(id)
      , m_desc(desc)
    {
    }

    Texture getPrimaryTexture()
    {
      return Texture(primaryImpl, id, m_desc);
    }

    Texture getSecondaryTexture()
    {
      return Texture(secondaryImpl, id, m_desc);
    }

    ResourceDescriptor& desc()
    {
      return *m_desc;
    }
  };
};