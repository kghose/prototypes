/*
 * Yes, we did it. We used a custom parser
 */

// TODO: rename this to simconfig.*

#pragma once

#include <string>
#include <vector>

#include "chrono/core/ChVector3.h"

using namespace chrono;

struct Part {
  double radius;
  ChVector3d pos;
};

struct Thrust {
  double start_s;
  double end_s;
  double force_N;
  ChVector3d loc;
  ChVector3d dir;
};

// TODO: rename to SimConfig
struct SimSetup {
  double end_s;
  double density;
  std::vector<Part> parts;
  std::vector<Thrust> thrust_actions;
};

SimSetup load_from(std::string filename);
