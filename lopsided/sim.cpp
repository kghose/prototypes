#include <vector>

#include "chrono/assets/ChVisualShapeCylinder.h"
#include "chrono/core/ChVector3.h"
#include "chrono/physics/ChBody.h"

#include "sim.hpp"

Sim::Sim(const SimFile &config) {

  auto create_cylinder_mp = [](double r, double h, double rho) {
    double mass = M_PI * r * r * h * rho;

    chrono::ChMatrix33d inertia;
    inertia.setZero();
    inertia(1, 1) = 0.5 * mass * r * r; // Iyy (spin axis)
    inertia(0, 0) = (1.0 / 12.0) * mass * (3 * r * r + h * h); // Ixx
    inertia(2, 2) = inertia(0, 0); // Izz (by symmetry)

    return chrono::ChMassProperties(mass, chrono::ChVector3d(0, 0, 0), inertia);
  };

  ship_ = chrono_types::make_shared<ChBody>();

  chrono::ChQuaterniond q_init(1, 0, 0, 0);

  chrono::CompositeInertia ship_inertia;
  for (const auto &part : config.ship_parts) {
    chrono::ChMassProperties mp =
        create_cylinder_mp(part.radius, part.length, part.density);
    ship_inertia.AddComponent(chrono::ChFramed(part.pos, q_init), mp.mass,
                              mp.inertia);
  }

  ship_->SetMass(ship_inertia.GetMass());
  ship_->SetInertia(ship_inertia.GetInertia());
  ship_->SetPos(chrono::ChVector3d(0, 0, 0));

  // body->SetCoordsys(ChCoordsysd(ChVector3d(0, 0, 0), QUNIT));

  ChVector3d center_of_mass = ship_inertia.GetCOM();
  for (const auto &part : config.ship_parts) {
    ship_->AddVisualShape(
        chrono_types::make_shared<chrono::ChVisualShapeCylinder>(part.radius,
                                                                 part.length),
        chrono::ChFramed(part.pos - center_of_mass, q_init));
  }

  for (auto &ta : config.thruster_actions) {

    auto f = chrono_types::make_shared<ChForce>();
    ship_->AddForce(f);
    f->SetMode(ChForce::ForceType::FORCE);
    f->SetFrame(ChForce::ReferenceFrame::BODY);
    f->SetVrelpoint(ta.loc);
    f->SetDir(ta.dir);

    ThrusterAction ta_(ta);
    ta_.force = f;
    thruster_actions_.push_back(ta_);
  }
}

std::shared_ptr<ChBody> Sim::ship() { return ship_; }

void Sim::apply_thrust_actions(double t_s) {
  for (auto &th : thruster_actions_) {
    if ((t_s > th.start_s) && (t_s <= th.end_s)) {
      th.force->SetMforce(th.force_N);
    } else {
      th.force->SetMforce(0);
    }
  }
}
