/*
 * Configuration file pattern

cam p 100 0 0 d 0 0 0
part d 500  r 2 l 20 p 100 0 0
part d 7000 r 4 l 10 p 0 0 0
thrust t0 0.2 t1 0.5 f 1000 p 0.1 0.4 6 d 1 0 0
part d 7000 r 1 l 15 p -5 0.4 6
thrust t0 0.2 t1 0.5 f 1000 p 0.1 0.4 6 d 1 0 0

*/

#include <fstream>
#include <sstream>

#include "simfile.hpp"

std::string strip_comments(const std::string &line) {
  size_t comment_pos = line.find('#');
  if (comment_pos != std::string::npos) {
    return line.substr(0, comment_pos);
  }
  return line;
}

SimFile load_simfile(std::string filename) {
  SimFile sim_config;
  std::ifstream file(filename);
  if (!file.is_open()) {
    throw std::runtime_error("Could not open file: " + filename);
  }

  std::string discard;
  double x, y, z;

  std::string raw_line, key;
  while (std::getline(file, raw_line)) {
    std::stringstream ss(strip_comments(raw_line));
    if (!(ss >> key))
      continue; // empty

    if (key == "cam") {
      // cam p 100 0 0 d 0 0 0
      ss >> discard >> x >> y >> z; 
      sim_config.camera.pos = ChVector3d(x, y, z);
      ss >> discard >> x >> y >> z; 
      sim_config.camera.target = ChVector3d(x, y, z);
    } else if (key == "part") {
      ShipPart p;

      // part d 500  r 2 l 20 p 100 0 0
      if (ss >> discard >> p.density >> discard >> p.radius >> discard >>
          p.length >> discard >> x >> y >> z) {
        p.pos = ChVector3d(x, y, z);
        sim_config.ship_parts.push_back(p);
      } else {
        throw std::runtime_error("Error parsing: " + raw_line);
      }
    } else if (key == "thrust") {
      ThrusterAction t;
      double px, py, pz, dx, dy, dz;

      // thrust t0 0.2 t1 0.5 f 1000 p 0.1 0.4 6 d 1 0 0
      if (ss >> discard >> t.start_s >> discard >> t.end_s >> discard >>
          t.force_N >> discard >> px >> py >> pz >> discard >> dx >> dy >> dz) {
        t.loc = ChVector3d(px, py, pz);
        t.dir = ChVector3d(dx, dy, dz);
        sim_config.thruster_actions.push_back(t);
      } else {
        throw std::runtime_error("Error parsing: " + raw_line);
      }
    }
  }

  return sim_config;
}
