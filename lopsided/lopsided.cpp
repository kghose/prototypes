#include "chrono/assets/ChVisualShapeCylinder.h"
#include "chrono/core/ChQuaternion.h"
#include "chrono/core/ChVector3.h"
#include "chrono/physics/ChBody.h"
#include "chrono/physics/ChBodyEasy.h"
#include "chrono/physics/ChMassProperties.h"
#include "chrono/physics/ChSystemNSC.h"
#include "chrono_irrlicht/ChVisualSystemIrrlicht.h"
#include <cmath>
#include <iostream>
#include <vector>

#include "simconfig.hpp"

using namespace chrono;
using namespace chrono::irrlicht;

/*
struct Cylinder {
  double length;
  double radius;
  double density;
  ChVector3d pos;
  ChQuaternion<> rot;
  ChColor color;

  double const mass() { return CH_PI * std::pow(radius, 2) * length * density; }
};

double total_mass_of(const std::map<std::string, Cylinder> &parts) {
  double mass = 0;
  for (const auto &[name, part] : parts) {
    mass += part.mass();
  }
  return mass;
}

std::shared_ptr<ChBody>
CreateStructure(const std::vector<const Cylinder> &cylinders,
                ChSystem *system) {

auto body = std::make_shared<ChBody>();
system->AddBody(body);

ChMassProperties total_mp;
for(const auto& cyl : cylinders) {
        ChMassProperties mp = ChMassProperties::Cylinder(ChAxis::Y, cyl.radius,
cyl.length, cyl.density);


}

int main() {
  // 1. Initialize the physical system
  ChSystemNSC sys;
  sys.SetGravitationalAcceleration(ChVector3d(0, 0, 0));

  // 2. Define geometry parameters
  auto parts = std::map<std::string, Cylinder>{
      {"cabin",     Cylinder{.length = 5.0,
                         .radius = 1.0,
                         .density = 500,
                         .pos = ChVector3d(0.0, 0.0, -2.0),
                         .rot = QuatFromAngleZ(0),
                         .color = ChColor(0.2f, 0.2f, 1.0f)}        },
      {"connector", Cylinder{.length = 2.0,
                             .radius = .05,
                             .density = 500,
                             .pos = ChVector3d(0.0, 0.0, -1.0),
                             .rot = QuatFromAngleY(-CH_PI / 2.0),
                             .color = ChColor(0.2f, 0.2f, 1.0f)}},
      {"main_bay",  Cylinder{.length = 3.0,
                            .radius = 2.0,
                            .density = 700,
                            .pos = ChVector3d(0.0, 0.0, -2.0),
                            .rot = QuatFromAngleZ(0),
                            .color = ChColor(0.2f, 1.0f, 0.2f)}  },
      {"equipment", Cylinder{.length = 3.0,
                             .radius = 1.0,
                             .density = 1000,
                             .pos = ChVector3d(0.0, 0.0, -2.0),
                             .rot = QuatFromAngleZ(0),
                             .color = ChColor(1.0f, 0.2f, 0.2f)}},
  };

  // 3. Create a single rigid body
  auto lopsided_ship = std::make_shared<ChBodyAuxRef>();
  sys.AddBody(lopsided_ship);

  lopsided_ship->SetMass(total_mass_of(parts));
  lopsided_ship->SetInertiaXX(ChVector3d(1.0, 1.0, 0.5));

  // 5. Attach the Visual Shapes
  ChQuaternion<> rod_rotation = QuatFromAngleY(-CH_PI / 2.0);

  // Central Rod
  auto rod_shape =
      std::make_shared<ChVisualShapeCylinder>(rod_radius, rod_length);
  rod_shape->SetColor(ChColor(0.7f, 0.7f, 0.7f));
  lopsided_ship->AddVisualShape(rod_shape,
                                ChFramed(ChVector3d(0, 0, 0), rod_rotation));

  // Left Cylinder
  auto left_cyl_shape =
      std::make_shared<ChVisualShapeCylinder>(cylinder_radius, cylinder_height);
  left_cyl_shape->SetColor(ChColor(0.2f, 0.4f, 0.8f));
  ChVector3d left_pos(-rod_length / 2.0, 0, 0);
  lopsided_ship->AddVisualShape(left_cyl_shape,
                                ChFramed(left_pos, rod_rotation));

  // Right Cylinder
  auto right_cyl_shape =
      std::make_shared<ChVisualShapeCylinder>(cylinder_radius, cylinder_height);
  right_cyl_shape->SetColor(ChColor(0.2f, 0.4f, 0.8f));
  ChVector3d right_pos(rod_length / 2.0, 0, 0);
  lopsided_ship->AddVisualShape(right_cyl_shape,
                                ChFramed(right_pos, rod_rotation));

  // 6. Apply initial kinematic conditions
  lopsided_ship->SetPos(ChVector3d(0, 0, 0));
  lopsided_ship->SetAngVelLocal(ChVector3d(0, 0, 3.14));

  // 7. Create the Irrlicht Visualization System
  auto vis = std::make_shared<ChVisualSystemIrrlicht>();
  vis->AttachSystem(&sys);
  vis->SetWindowSize(1024, 768);
  vis->SetWindowTitle("Spinning Rigid Body - Project Chrono");
  vis->Initialize();

  // 8. Add camera, lighting, and optional grid lines
  vis->AddLogo();
  // vis->AddSkyBox();
  vis->AddCamera(ChVector3d(0, 3, -4),
                 ChVector3d(0, 0, 0)); // Camera at (0,3,-4) looking at origin
  vis->AddTypicalLights();

  // 9. Simulation Loop integrated with Visualization window
  double time_step = 0.01;

  std::cout << "Starting visualized simulation loop...\n";

  // vis->Run() checks if the window is closed by the user
  while (vis->Run()) {
    vis->BeginScene();
    vis->Render(); // Renders the visual shapes attached to the bodies
    vis->EndScene();

    sys.DoStepDynamics(time_step);
  }

  return 0;
}
*/

int main(int argc, char *argv[]) {
  ChSystemNSC sys;
  // sys.SetGravitationalAcceleration(ChVector3d(0, 0.0, 0));

  auto sim_config = SimConfig::configure_sim();
  sys.Add(sim_config->get_body());

  // 5. Set up Irrlicht
  auto vis = chrono_types::make_shared<ChVisualSystemIrrlicht>();
  vis->AttachSystem(&sys);
  vis->SetWindowSize(1024, 768);
  vis->SetWindowTitle("Lopsided spinning ship with thrust");

  // XZ grid
  ChQuaterniond grid_rotation = QuatFromAngleX(CH_PI_2);
  ChCoordsysd grid_frame(ChVector3d(0, 0, 0), grid_rotation);

  // Parameters: step_x, step_z, num_divisions_x, num_divisions_z, frame, color
  vis->AddGrid(10, 10, 100, 100, grid_frame, ChColor(0.3f, 0.3f, 0.3f));
  vis->EnableAbsCoordsysDrawing(true);

  vis->Initialize();
  // vis->AddLogo();
  // vis->AddSkyBox();
  vis->AddTypicalLights();
  // vis->AddCamera(ChVector3d(0, 4, -6), ChVector3d(0, 1, 0));
  vis->AddCamera(ChVector3d(0, 20, -20), ChVector3d(0, 1, 0));

  // 6. Simulation loop
  double time_step = 0.01;

  while (vis->Run()) {
    vis->BeginScene();
    vis->Render();
    vis->RenderCOGFrames(
        5.0); // 0.5 dictates the line length of the drawn axes in meters
    vis->EndScene();

    sim_config->apply_thrust_actions(sys.GetChTime());

    sys.DoStepDynamics(time_step);
  }

  return 0;
}
