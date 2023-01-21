#pragma once

#include <memory>
#include <tuple>
#include <utility>

#include <entt/core/type_traits.hpp>
#include <entt/entity/registry.hpp>

template<typename T>
void update(T& system, const float delta_t);

template<typename...>
struct System_info;
template<typename ObserversT, typename... ReadTs, typename... WriteTs>
struct System_info<ObserversT, entt::type_list<ReadTs...>, entt::type_list<WriteTs...>>{

  std::unique_ptr<ObserversT> observers;

  using read_type_list = entt::type_list<ReadTs...>;
  using write_type_list = entt::type_list<WriteTs...>;
  std::tuple<const entt::sigh_storage_mixin<entt::storage<ReadTs>>&...> readStorages;
  std::tuple<entt::sigh_storage_mixin<entt::storage<WriteTs>>&...> writeStorage;
};

template <typename SystemInfoT, typename ObserversT, typename... ReadTs, typename... WriteTs>
auto make_system_info(entt::registry& r, entt::type_list<ReadTs...>&&, entt::type_list<WriteTs...>&&) {
  return SystemInfoT{ std::make_unique<ObserversT>(r), {r.storage<ReadTs>()...}, { r.storage<WriteTs>()...} };
}

template<typename T>
auto get_read_storage(auto&& system_info) -> const entt::sigh_storage_mixin<entt::storage<T>>&
{
  return std::get<const entt::sigh_storage_mixin<entt::storage<T>>&>(system_info.readStorages);
}

template<typename T>
auto get_write_storage(auto&& system_info) -> entt::sigh_storage_mixin<entt::storage<T>>&
{
  return std::get<entt::sigh_storage_mixin<entt::storage<T>>&>(system_info.writeStorage);
}

struct System{
private:
  struct System_concept
  {
      virtual ~System_concept(){}
      virtual void update(const float delta_t) = 0;
  };

  template<typename SystemInfoT>
  struct System_model : public System_concept
  {
    explicit System_model(SystemInfoT&& system_info)
      : system_info{ std::move(system_info) } {}

    void update(const float delta_t) override
    {
      ::update(system_info, delta_t);
    }

    template<typename T>
    auto get_read_storage() -> const entt::sigh_storage_mixin<entt::storage<T>>&
    {
      return get_read_storage<T>(system_info);
    }

    template<typename T>
    auto get_write_storage() -> entt::sigh_storage_mixin<entt::storage<T>>&
    {
      return get_write_storage<T>(system_info);
    }

    SystemInfoT system_info;
  };

public:
  template<typename SystemInfoT>
  System(SystemInfoT&& system_info)
  : p_impl{std::make_unique<System_model<SystemInfoT>>(std::move(system_info))}
  {}
  
  void update(const float delta_t)
  {
    p_impl->update(delta_t);
  }

private:
  std::unique_ptr<System_concept> p_impl;
};
