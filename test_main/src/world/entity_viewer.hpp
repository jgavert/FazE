#pragma once
#include <higanbana/core/entity/database.hpp>

namespace app
{
class EntityView
{
  bool m_show = false;
  public:
    void render(higanbana::Database<2048>& ecs);
};
};