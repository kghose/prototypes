#include <vector>

#include "chrono/core/ChVector3.h"
#include "chrono/physics/ChBodyEasy.h"

#include "simconfig.hpp"
#include "simfile.hpp"

// TODO: Rename this as SimBody

struct Force {
  double start_s;
  double end_s;
  double force_N;
  std::shared_ptr<ChForce> force;
};

class ClusterSimConfig : public SimConfig {
private:
  std::shared_ptr<ChBodyEasyClusterOfSpheres> body;
  // std::vector<Thrust> thrust_actions;
  // std::vector<std::shared_ptr<ChForce>> forces;
  std::vector<Force> forces;

public:
  ClusterSimConfig() {
    SimSetup this_sim = load_from("simconfig.txt");

    std::vector<ChVector3d> sphere_pos;
    std::vector<double> sphere_radii;
    for (const auto &part : this_sim.parts) {
      sphere_pos.push_back(part.pos);
      sphere_radii.push_back(part.radius);
    }

    body = chrono_types::make_shared<ChBodyEasyClusterOfSpheres>(
        /*positions=*/sphere_pos,
        /*radii=*/sphere_radii,
        /*density=*/this_sim.density,
        /*create_visualization=*/true
        ///*create_collision=*/false,
        ///*material=*/chrono_types::make_shared<ChContactMaterialNSC>()
    );

    body->SetCoordsys(ChCoordsysd(ChVector3d(0, 0, 0), QUNIT));

    for (const auto &thrust : this_sim.thrust_actions) {
      auto f = chrono_types::make_shared<ChForce>();
      body->AddForce(f);
      f->SetMode(ChForce::ForceType::FORCE);
      f->SetFrame(ChForce::ReferenceFrame::BODY);
      f->SetVrelpoint(thrust.loc);
      f->SetDir(thrust.dir);

      forces.push_back(Force{
          .start_s = thrust.start_s,
          .end_s = thrust.end_s,
          .force_N = thrust.force_N,
          .force = f,
      });
    }
  }

  std::shared_ptr<ChBodyEasyClusterOfSpheres> get_body() override {
    return body;
  }

  void apply_thrust_actions(double t_s) override {
    for (auto &f : forces) {
      if ((t_s > f.start_s) && (t_s <= f.end_s)) {
        f.force->SetMforce(f.force_N);
      } else {
        f.force->SetMforce(0);
      }
    }
  }
};

std::unique_ptr<SimConfig> SimConfig::configure_sim() {
  return std::make_unique<ClusterSimConfig>();
}
