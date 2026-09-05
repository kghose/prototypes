/*
 * We keep the body and force definitions in a separate compilation unit so
 * that we can compile fast when we only change the body and force
 * configurations and can enventually replace it with a loadable simulation data
 * file.
 */
#pragma once
#include "chrono/physics/ChBodyEasy.h"

using namespace chrono;

class SimConfig {
public:
  virtual ~SimConfig() = default;
  virtual double get_end_s() = 0;
  virtual std::shared_ptr<ChBodyEasyClusterOfSpheres> get_body() = 0;
  virtual void apply_thrust_actions(double t_s) = 0;

  static std::unique_ptr<SimConfig> configure_sim();
};
