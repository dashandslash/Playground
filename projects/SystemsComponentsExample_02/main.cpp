#include <filesystem>
#include <type_traits>
#include <tuple>

#include <entt/core/type_traits.hpp>
#include <entt/entity/registry.hpp>
#include <entt/entity/observer.hpp>
#include <imgui/imgui.h>

#include <blackboard_app/app.h>
#include <blackboard_app/gui.h>
#include <blackboard_app/resources.h>
#include <blackboard_app/window.h>


#include "system_a.h"
#include "system_b.h"
#include "system_definition.h"

using Callback_data = entt::dense_map<entt::entity, entt::dense_map<entt::id_type, std::pair<std::string, uint32_t>>>;

template <typename T>
void some_callback(entt::registry &r, entt::entity e)
{
  auto& callback_data{r.ctx().get<Callback_data>()};
  auto type_info{entt::type_id<T>()};

  auto& map = callback_data[e];
  auto& [text, total] = map[type_info.hash()];

  if(text.empty())
  {
    text = std::string{"Callback - Type: ["};
    text.append(std::string{entt::type_id<T>().name()});
    text.append("] created or modified on entity: [");
    text.append(std::to_string(entt::to_integral(e)));
    text.append("]");
  }
  
  total++;
}

std::unique_ptr<blackboard::app::App> app;

struct Context {
  entt::registry r{};
  System system_a = make_system_info<System_A_info>(r);
  System system_b = make_system_info<System_B_info>(r);
};

void execute_systems(Context& ctx)
{
  ctx.system_a.update(blackboard::app::App::delta_time());
  ctx.system_b.update(blackboard::app::App::delta_time());
}

void init(Context& ctx)
{
  blackboard::app::gui::set_blackboard_theme();
  const auto dpi{app->main_window.get_ddpi()};
  blackboard::app::gui::load_font(blackboard::app::resources::path() / "assets/fonts/Inter/Inter-Light.otf", 12.0f,
                                  dpi);
  auto& r = ctx.r;
  r.ctx().emplace<Callback_data>();
  // There are two ways to register a listener on a specific component
  // Using the sigh storage mixin
  r.storage<C1>().on_construct().connect<some_callback<C1>>();
  r.storage<C1>().on_update().connect<some_callback<C1>>();
  // Or by using the registry, the result is the same.
  r.on_construct<C2>().connect<some_callback<C2>>();
  r.on_update<C2>().connect<some_callback<C2>>();
  
  r.on_construct<C3>().connect<some_callback<C3>>();
  r.on_update<C3>().connect<some_callback<C3>>();
}

void update_ui(Context& ctx)
{
  blackboard::app::gui::dockspace();
  ImGui::Begin("Demo");
  
  ImGui::TextColored({1.0, 0.0, 0.0, 1.0}, "System_A: C2 = C1 + 1.0f");
  ImGui::TextColored({1.0, 0.0, 0.0, 1.0}, "System_B: C3 = C1 + C2");

  auto& r = ctx.r;
  auto &storageC1 = r.storage<C1>();
  auto &storageC2 = r.storage<C2>();
  auto &storageC3 = r.storage<C3>();
  
  static float c1_value{0.0f};
  
  ImGui::PushItemWidth(80.0f);
  ImGui::InputFloat("C1 value", &c1_value);
  ImGui::PopItemWidth();
  ImGui::SameLine();

  if(ImGui::Button("Create entity"))
  {
    storageC1.emplace(r.create(), c1_value);
  }
  
  entt::basic_view c1_c2_c3_view{storageC1, storageC2, storageC3};
  for(auto [e, c1, c2, c3] : c1_c2_c3_view.each())
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
    ImGui::SameLine();
    ImGui::Text("C3 value %f", c3.v);
    ImGui::PopID();
  }
  ImGui::End();
  
  ImGui::Begin("Callbacks");
  const auto& callback_data{r.ctx().get<Callback_data>()};

  for(auto&& [e, map] : callback_data)
  {
    ImGui::PushID(entt::to_integral(e));
    for (auto&& [hash, info] : map) {
      const auto& [text, total] = info;
      ImGui::PushID(entt::to_integral(hash));
      ImGui::Text("Entity %i", entt::to_integral(e));
      ImGui::SameLine();
      ImGui::Text("%s", text.c_str());
      ImGui::SameLine();
      ImGui::Text("Total callbacks: %i", total);
      ImGui::PopID();
    }
    ImGui::PopID();
  }
  ImGui::End();
}

void app_update(Context& ctx)
{
  // UI
  update_ui(ctx);

  // Execute the systems
  execute_systems(ctx);
}

int main(int argc, char *argv[])
{
  app = std::make_unique<blackboard::app::App>("SystemsComponentsExample_02", blackboard::app::renderer::Api::AUTO);

  Context ctx{};
  app->on_update = [&ctx](){app_update(ctx);};
  app->on_init = [&ctx](){init(ctx);};
  app->run();

  app.reset();
  return 0;
}
