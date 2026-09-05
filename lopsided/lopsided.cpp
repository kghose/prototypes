#include "chrono/core/ChQuaternion.h"
#include "chrono/core/ChVector3.h"
#include "chrono/physics/ChSystemNSC.h"
#include "chrono_irrlicht/ChVisualSystemIrrlicht.h"

#include "simconfig.hpp"

using namespace chrono;
using namespace chrono::irrlicht;

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
