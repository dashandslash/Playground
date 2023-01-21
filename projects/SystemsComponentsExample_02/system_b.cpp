#pragma once
#include "system_b.h"

#include <tuple>
#include <utility>

#include <entt/core/type_traits.hpp>
#include <entt/entity/registry.hpp>
#include <entt/entity/observer.hpp>

#include "components.h"
#include "system_definition.h"


template<>
void update(System_B_info& system, const float delta_t)
{
  for(const auto e : system.observers->obs)
  {
    auto& storageC3{get_write_storage<C3>(system)};
    
    auto& c1 = get_read_storage<C1>(system).get(e);
    auto& c2 = get_read_storage<C2>(system).get(e);

    if(storageC3.contains(e))
    {
      storageC3.patch(e, [&](auto& c3){
        c3.v = c1.v + c2.v;
      });
    }
    else
    {
      storageC3.emplace(e, c1.v + c2.v);
    }
  }
  
  system.observers->obs.clear();
}
