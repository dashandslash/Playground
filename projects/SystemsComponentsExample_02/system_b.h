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

using System_B_info = System_info<System_B_observer, System_B_read_typelist, System_B_write_typelist>;
