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

  using Observers = ObserversT;
  using Read_type_list = entt::type_list<ReadTs...>;
  using Write_type_list = entt::type_list<WriteTs...>;
  std::unique_ptr<Observers> observers;
  std::tuple<const entt::sigh_storage_mixin<entt::storage<ReadTs>>&...> readStorages;
  std::tuple<entt::sigh_storage_mixin<entt::storage<WriteTs>>&...> writeStorage;
  
  std::vector<entt::type_info> read_types_info;
  std::vector<entt::type_info> write_types_info;
};

namespace internal {
  template<typename...>
  struct make_system;
  template <typename SystemInfoT, typename... ReadTs, typename... WriteTs>
  struct make_system<SystemInfoT, entt::type_list<ReadTs...>, typename entt::type_list<WriteTs...>> {
    inline static SystemInfoT create(entt::registry& r)
    {
      return SystemInfoT{ .observers = std::make_unique<typename SystemInfoT::Observers>(r), .readStorages = {r.storage<ReadTs>()...}, .writeStorage = {r.storage<WriteTs>()...},
        .read_types_info = {entt::type_id<ReadTs>()...}, .write_types_info = {entt::type_id<WriteTs>()...}
      };
    }
  };
}

template <typename SystemInfoT>
SystemInfoT make_system(entt::registry& r) {
  return internal::make_system<SystemInfoT, typename SystemInfoT::Read_type_list, typename SystemInfoT::Write_type_list>::create(r);
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
      virtual const std::vector<entt::type_info>& read_types_info() const = 0;
      virtual const std::vector<entt::type_info>& write_types_info() const = 0;
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
    
    const std::vector<entt::type_info>& read_types_info() const override
    {
      return system_info.read_types_info;
    }
    
    const std::vector<entt::type_info>& write_types_info() const override
    {
      return system_info.write_types_info;
    }

    SystemInfoT system_info;
  };

public:
  template<typename SystemInfoT>
  System(SystemInfoT&& system_info)
  : type_info{entt::type_id<SystemInfoT>()}, p_impl{std::make_unique<System_model<SystemInfoT>>(std::move(system_info))}
  {}
  
  void update(const float delta_t)
  {
    p_impl->update(delta_t);
  }
  
  const std::vector<entt::type_info>& read_type_info() const
  {
    return p_impl->read_types_info();
  }
  
  const std::vector<entt::type_info>& write_type_info() const
  {
    return p_impl->write_types_info();
  }
  

  entt::type_info type_info;

private:
  std::unique_ptr<System_concept> p_impl;
};
