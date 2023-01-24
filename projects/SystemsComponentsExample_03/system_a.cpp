#pragma once
#include "system_a.h"

#include <tuple>
#include <utility>

#include <entt/core/type_traits.hpp>
#include <entt/entity/registry.hpp>
#include <entt/entity/observer.hpp>

#include "components.h"
#include "system_definition.h"

// This system sums 1.0f to C1 and stores it into C2
template<>
void update(System_A_info& system, const float delta_t)
{
  for(const auto e : system.observers->obs)
  {
    auto& storageC2{get_write_storage<C2>(system)};
    
    auto& c1 = get_read_storage<C1>(system).get(e);

    if(storageC2.contains(e))
    {
      storageC2.patch(e, [&](auto& c2){
        c2.v = c1.v + 1.0f;
      });
    }
    else
    {
      storageC2.emplace(e, c1.v + 1.0f);
    }
  }
  
  system.observers->obs.clear();
}
