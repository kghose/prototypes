/*
 * Yes, we did it. We used a custom parser
 */

#pragma once

#include <string>
#include <vector>

#include "chrono/core/ChVector3.h"
#include <chrono/physics/ChForce.h>

using namespace chrono;

struct Camera {
  ChVector3d pos;
  ChVector3d target;
};

struct ShipPart {
  double density;
  double radius;
  double length;
  ChVector3d pos;
  ChVector3d dir;
};

struct ThrusterAction {
  double start_s;
  double end_s;
  double force_N;
  ChVector3d loc;
  ChVector3d dir;
  std::shared_ptr<ChForce> force;
};

struct SimFile {
  double end_s;
  Camera camera;
  std::vector<ShipPart> ship_parts;
  std::vector<ThrusterAction> thruster_actions;
};

SimFile load_simfile(std::string filename);
