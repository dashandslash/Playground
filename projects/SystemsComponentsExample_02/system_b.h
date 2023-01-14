#pragma once

#include <tuple>
#include <utility>

#include <entt/core/type_traits.hpp>
#include <entt/entity/registry.hpp>
#include <entt/entity/observer.hpp>

#include "components.h"
#include "system_definition.h"


// This system sums C1 and C2 and store the result in C3

struct System_B_observer
{
  System_B_observer(entt::registry& r) : obs{r, entt::collector.group<C1, C2>().update<C1>().where<C2>().update<C2>().where<C1>()}{}
  entt::observer obs;
};

using System_B_read_typelist = entt::type_list<C1, C2>;
using System_B_write_typelist = entt::type_list<C3>;

using System_B = System::System_model<System_B_observer, System_B_read_typelist, System_B_write_typelist>;

template<> void update(System_B& system, float)
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
