cmake_minimum_required(VERSION 3.10)
project(sim CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find the Chrono package and any required modules (e.g., Irrlicht for 3D UI)
find_package(Chrono REQUIRED COMPONENTS Irrlicht)

add_library(
	SimConfigLib
	simconfig.cpp
	simconfig.hpp
	simfile.cpp
	simfile.hpp
)
target_link_libraries(
	SimConfigLib
	Chrono::Chrono_core
) 

add_executable(sim main.cpp)
target_link_libraries(
	sim 
	Chrono::Chrono_core 
	Chrono::Chrono_irrlicht 
	SimConfigLib
)

