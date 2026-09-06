/*
 * Configuration file pattern

density 1000
part r 0.5 p 0.1 0.4 6
part r 0.5 p 0.1 0.4 6
thrust t0 0.2 t1 0.5 f 1000 p 0.1 0.4 6 d 1 0 0
part r 0.5 p 0.1 0.4 6
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

SimSetup load_from(std::string filename) {
  SimSetup sim_config;
  std::ifstream file(filename);
  if (!file.is_open()) {
    throw std::runtime_error("Could not open file: " + filename);
  }

  std::string discard;

  std::string raw_line, key;
  while (std::getline(file, raw_line)) {
    std::stringstream ss(strip_comments(raw_line));
    if (!(ss >> key))
      continue; // empty

    if (key == "density") {
      ss >> sim_config.density;
    } else if (key == "part") {
      Part p;
      double x, y, z;

      // Expected format: part r 0.5 p 0.1 0.4 6
      if (ss >> discard >> p.radius >> discard >> x >> y >> z) {
        p.pos = ChVector3d(x, y, z);
        sim_config.parts.push_back(p);
      } else {
	      throw std::runtime_error("Error parsing: " + raw_line);
      }
    } else if (key == "thrust") {
      Thrust t;
      double px, py, pz, dx, dy, dz;

      // Expected format: thrust t0 0.2 t1 0.5 f 1000 p 0.1 0.4 6 d 1 0 0
      if (ss >> discard >> t.start_s >> discard >> t.end_s >> discard >>
          t.force_N >> discard >> px >> py >> pz >> discard >> dx >> dy >> dz) {
        t.loc = ChVector3d(px, py, pz);
        t.dir = ChVector3d(dx, dy, dz);
        sim_config.thrust_actions.push_back(t);
      } else {
	      throw std::runtime_error("Error parsing: " + raw_line);
      }
    }
  }

  return sim_config;
}
