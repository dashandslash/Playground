#include <filesystem>
#include <tuple>
#include <type_traits>

#include <entt/core/type_traits.hpp>
#include <entt/entity/registry.hpp>
#include <imgui/imgui.h>

#include <blackboard_app/app.h>
#include <blackboard_app/gui.h>
#include <blackboard_app/resources.h>
#include <blackboard_app/window.h>

struct C1 {
  float v{};
};

struct C2 {
  float v{};
};

template<typename...>
struct System;

template<typename... ReadTs, typename... WriteTs>
struct System<entt::type_list<ReadTs...>, entt::type_list<WriteTs...>>{

using read_type_list = entt::type_list<ReadTs...>;
using write_type_list = entt::type_list<WriteTs...>;

std::tuple<const entt::sigh_storage_mixin<entt::storage<ReadTs>>&...> readStorages;
std::tuple<entt::sigh_storage_mixin<entt::storage<WriteTs>>&...> writeStorage;
};

namespace internal {

template<typename C, typename... Ts>
auto add_update(C& c)
{
  return add_update(c.template update<Ts>(c)...);
}

template<typename SystemT, typename... ReadTs, typename... WriteTs>
auto make_system(entt::registry& r, entt::type_list<ReadTs...>&&, entt::type_list<WriteTs...>&&) -> SystemT
{
  return SystemT{.readStorages = {r.storage<ReadTs>()...}, .writeStorage = {r.storage<WriteTs>()...}};
}
}

template<typename SystemT>
auto make_system(entt::registry& r) -> SystemT
{
  return internal::make_system<SystemT>(r, typename SystemT::read_type_list{}, typename SystemT::write_type_list{});
}

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

using ADefinedSystem = System<entt::type_list<C1>, entt::type_list<C2>>;

void execute(ADefinedSystem& system)
{
  entt::basic_view c1_view{get_read_storage<C1>(system)};
  
  for(const auto [e, c1] : c1_view.each())
  {
    auto& storageC2{get_write_storage<C2>(system)};
    
    auto e_int = (int)e;
    
    if(storageC2.contains(e))
    {
      storageC2.patch(e, [&](auto& c2){
        c2.v += 1.0f;
      });
    }
    else
    {
      storageC2.emplace(e, c1.v);
    }
  }
}

template <typename T>
void some_callback(entt::registry &r, entt::entity e)
{
  std::cout << "Callback - Type: [" << entt::type_id<T>().name()  << "] created or modified on entity: [" << entt::to_integral(e) << "]"<< std::endl;
}

struct Context {
  blackboard::app::App &app;
  entt::registry r;
  ADefinedSystem definedSystem = make_system<ADefinedSystem>(r);
};

void init(Context& ctx)
{
  auto& app = ctx.app;
  
  blackboard::app::gui::set_blackboard_theme();
  const auto dpi{app.main_window.get_ddpi()};
  blackboard::app::gui::load_font(blackboard::app::resources::path() / "assets/fonts/Inter/Inter-Light.otf", 12.0f,
                                  dpi);
  auto& r = ctx.r;
  // There are two ways to register a listener on a specific component
  // Using the sigh storage mixin
  r.storage<C1>().on_construct().connect<some_callback<C1>>();
  r.storage<C1>().on_update().connect<some_callback<C1>>();
  // Or by using the registry, the result is the same.
  r.on_construct<C2>().connect<some_callback<C2>>();
  r.on_update<C2>().connect<some_callback<C2>>();

  auto &storageC1 = r.storage<C1>();
  storageC1.emplace(r.create(), 0.0f);
}

void update_ui(Context& ctx)
{
  blackboard::app::gui::dockspace();
  ImGui::Begin("Demo");
  
  auto& r = ctx.r;
  auto &storageC1 = r.storage<C1>();
  auto &storageC2 = r.storage<C2>();
  
  if(ImGui::Button("Add"))
  {
    storageC1.emplace(r.create(), (float)(std::rand()%100u));
  }
  
  entt::basic_view c1_c2_view{storageC1, storageC2};
  for(auto [e, c1, c2] : c1_c2_view.each())
  {
    ImGui::PushID(entt::to_integral(e));
    ImGui::Text("Entity %i", entt::to_integral(e));
    ImGui::SameLine();
    ImGui::Text("C1 value");
    ImGui::SameLine();
    ImGui::PushItemWidth(80.0f);
    if(ImGui::InputFloat("##C1_value", &c1.v))
    {
      // Calling patch triggers the callback.
      // There are better way to edit a component,
      // but for the sake of the current example this is just fine
      r.patch<C1>(e, [](auto&...){});
    }
    ImGui::PopItemWidth();
    ImGui::SameLine();
    ImGui::Text("C2 value %f", c2.v);
    ImGui::PopID();
  }
  ImGui::End();
}

void app_update(Context& ctx)
{
  // Execute the systems
  execute(ctx.definedSystem);

  // UI
  update_ui(ctx);
}

int main(int argc, char *argv[])
{
  blackboard::app::App blackboardApp{"SystemsComponentsExample_01", blackboard::app::renderer::Api::AUTO};
  Context ctx{.app = blackboardApp};
  blackboardApp.on_update = [&ctx](){app_update(ctx);};
  blackboardApp.on_init = [&ctx](){init(ctx);};
  blackboardApp.run();

  return 0;
}
