#pragma once

#include <tuple>
#include <utility>

#include <entt/core/type_traits.hpp>
#include <entt/entity/registry.hpp>
#include <entt/entity/observer.hpp>

#include "components.h"
#include "system_definition.h"

// This system sums 1.0f to C1 and stores it into C2

struct System_A_observer
{
  System_A_observer(entt::registry& r) : obs{r, entt::collector.group<C1>().update<C1>()}{}
  entt::observer obs;
};

using System_A_read_typelist = entt::type_list<C1>;
using System_A_write_typelist = entt::type_list<C2>;

using System_A = System::System_model<System_A_observer, System_A_read_typelist, System_A_write_typelist>;

template<> void update(System_A& system, float)
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
