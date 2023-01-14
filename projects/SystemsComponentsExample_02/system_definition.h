#pragma once

#include <memory>
#include <tuple>
#include <utility>

#include <entt/core/type_traits.hpp>
#include <entt/entity/registry.hpp>

template<typename T>
void update(T&& system, const float delta_t);

struct System{
private:
  struct System_concept
  {
      virtual ~System_concept(){}
      virtual void update(const float delta_t) = 0;
  };

public:
  template<typename...>
  struct System_model;
  template<typename ObsT, typename... ReadTs, typename... WriteTs>
  struct System_model<ObsT, entt::type_list<ReadTs...>, entt::type_list<WriteTs...>> : public System_concept
  {
    explicit System_model(entt::registry& r, std::unique_ptr<ObsT>&& obs)
        : readStorages{r.storage<ReadTs>()...}, writeStorage{r.storage<WriteTs>()...}, observers{std::move(obs)}
    {}

    void update(const float delta_t) override
    {
      ::update(*this, delta_t);
    }
    
    std::unique_ptr<ObsT> observers;
        
    using read_type_list = entt::type_list<ReadTs...>;
    using write_type_list = entt::type_list<WriteTs...>;
    std::tuple<const entt::sigh_storage_mixin<entt::storage<ReadTs>>&...> readStorages;
    std::tuple<entt::sigh_storage_mixin<entt::storage<WriteTs>>&...> writeStorage;
  };
  
  template<typename ObsT, typename... ReadTs, typename... WriteTs>
  System(entt::registry& r, std::unique_ptr<ObsT>&& obs, entt::type_list<ReadTs...>&&, entt::type_list<WriteTs...>&&)
  : p_impl{std::make_unique<System_model<ObsT, entt::type_list<ReadTs...>, entt::type_list<WriteTs...>>>(r, std::move(obs))}
  {}
  
  void update(const float delta_t)
  {
    p_impl->update(delta_t);
  }

private:
  std::unique_ptr<System_concept> p_impl;
};

template<typename T>
auto get_read_storage(auto&& system) -> const entt::sigh_storage_mixin<entt::storage<T>>&
{
  return std::get<const entt::sigh_storage_mixin<entt::storage<T>>&>(system.readStorages);
}

template<typename T>
auto get_write_storage(auto&& system) -> entt::sigh_storage_mixin<entt::storage<T>>&
{
  return std::get<entt::sigh_storage_mixin<entt::storage<T>>&>(system.writeStorage);
}
