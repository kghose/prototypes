#include "chrono/core/ChQuaternion.h"
#include "chrono/core/ChVector3.h"
#include "chrono/physics/ChSystemNSC.h"
#include "chrono_irrlicht/ChVisualSystemIrrlicht.h"

#include "simconfig.hpp"

using namespace chrono;
using namespace chrono::irrlicht;

int main(int argc, char *argv[]) {
  ChSystemNSC sys;

  auto sim_ship = Ship::configure_sim();
  sys.Add(sim_ship->get_body());

  auto vis = chrono_types::make_shared<ChVisualSystemIrrlicht>();
  vis->AttachSystem(&sys);
  vis->SetWindowSize(1024, 768);
  vis->SetWindowTitle("Lopsided spinning ship with thrust");

  // XZ grid
  ChQuaterniond grid_rotation = QuatFromAngleX(CH_PI_2);
  ChCoordsysd grid_frame(ChVector3d(0, 0, 0), grid_rotation);
  vis->AddGrid(
      /*step_x=*/10,
      /*step_z*/ 10,
      /*num_divisions_x=*/100,
      /*num_divisions_z=*/100,
      /*frame=*/grid_frame,
      /*color=*/ChColor(0.3f, 0.3f, 0.3f));
  
  vis->EnableAbsCoordsysDrawing(true);

  vis->Initialize();
  vis->AddTypicalLights();
  
  ChVector3d cam_offset(10, 10, 0);

  vis->AddCamera(ChVector3d(0, 20, -20), ChVector3d(0, 1, 0));

  // 6. Simulation loop
  double time_step = 0.01;

  while (vis->Run()) {
    vis->BeginScene();
    vis->Render();
    vis->RenderCOGFrames(
        5.0); // 0.5 dictates the line length of the drawn axes in meters
    ChVector3d obj_pos = sim_ship->get_body()->GetPos();
    vis->UpdateCamera(obj_pos + cam_offset, obj_pos);
    vis->EndScene();

    sim_ship->apply_thrust_actions(sys.GetChTime());

    sys.DoStepDynamics(time_step);
  }

  return 0;
}
