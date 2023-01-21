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

using System_A_info = System_info<System_A_observer, System_A_read_typelist, System_A_write_typelist>;
