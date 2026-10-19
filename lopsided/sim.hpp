/*
 * We keep the body and force definitions in a separate compilation unit so
 * that we can compile fast when we only change the body and force
 * configurations and can enventually replace it with a loadable simulation data
 * file.
 */

#pragma once
#include "chrono/physics/ChBody.h"

#include "simfile.hpp"

using namespace chrono;

class Sim {
  std::shared_ptr<ChBody> ship_;
  std::vector<ThrusterAction> thruster_actions_;

public:
  Sim(const SimFile &);
  std::shared_ptr<ChBody> ship();
  void apply_thrust_actions(double t_s);
};
