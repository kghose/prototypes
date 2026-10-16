#include <vector>

#include "chrono/assets/ChVisualShapeCylinder.h"
#include "chrono/core/ChVector3.h"
#include "chrono/physics/ChBody.h"

#include "ship.hpp"
#include "simfile.hpp"

struct Force {
  double start_s;
  double end_s;
  double force_N;
  std::shared_ptr<ChForce> force;
};

class ShipImpl : public Ship {
private:
  std::shared_ptr<ChBody> body;
  // std::vector<Thrust> thrust_actions;
  // std::vector<std::shared_ptr<ChForce>> forces;
  std::vector<Force> forces;

public:
  ShipImpl(std::string fname) {
    SimSetup this_sim = load_from(fname);

    std::vector<ChVector3d> sphere_pos;
    std::vector<double> sphere_radii;

    // Helper lambda to instantly build a ChMassProperties struct for a Y-axis
    // cylinder
    auto create_cylinder_mp = [](double r, double h, double rho) {
      double mass = M_PI * r * r * h * rho;

      chrono::ChMatrix33d inertia;
      inertia.setZero();
      inertia(1, 1) = 0.5 * mass * r * r; // Iyy (spin axis)
      inertia(0, 0) = (1.0 / 12.0) * mass * (3 * r * r + h * h); // Ixx
      inertia(2, 2) = inertia(0, 0); // Izz (by symmetry)

      return chrono::ChMassProperties(mass, chrono::ChVector3d(0, 0, 0),
                                      inertia);
    };

    body = chrono_types::make_shared<ChBody>();

    float l = 10;
    chrono::ChQuaterniond q_init(1, 0, 0, 0);

    chrono::CompositeInertia ship_inertia;
    for (const auto &part : this_sim.parts) {
      chrono::ChMassProperties mp =
          create_cylinder_mp(part.radius, l, this_sim.density);
      ship_inertia.AddComponent(chrono::ChFramed(part.pos, q_init), mp.mass,
                                mp.inertia);
    }

    body->SetMass(ship_inertia.GetMass());
    body->SetInertia(ship_inertia.GetInertia());
    body->SetPos(chrono::ChVector3d(0, 0, 0));

    // body->SetCoordsys(ChCoordsysd(ChVector3d(0, 0, 0), QUNIT));

    ChVector3d center_of_mass = ship_inertia.GetCOM();
    for (const auto &part : this_sim.parts) {
      body->AddVisualShape(
          chrono_types::make_shared<chrono::ChVisualShapeCylinder>(part.radius,
                                                                   l),
          chrono::ChFramed(part.pos - center_of_mass, q_init));
    }

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

  std::shared_ptr<ChBody> get_body() override { return body; }

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

std::unique_ptr<Ship> Ship::configure_sim(std::string fname) {
  return std::make_unique<ShipImpl>(fname);
}
