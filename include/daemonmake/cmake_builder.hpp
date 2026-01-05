#ifndef DAEMONMAKE__DAEMONMAKE_CMAKE_BUILDER
#define DAEMONMAKE__DAEMONMAKE_CMAKE_BUILDER

#include "daemonmake/config.hpp"
#include "daemonmake/project.hpp"

namespace daemonmake {

/**
 * Configures and builds the project via CMake.
 *
 * Ensures the build directory exists, generates a CMakeLists.txt if missing,
 * runs a CMake configure step, then builds the project. Returns the exit code
 * from the build command. Does not throw on configuration or build failures.
 *
 * @param cfg       Project configuration, including project_root and build_directory.
 * @param pl        Project layout used when generating CMakeLists.txt.
 * @param overwrite Whether to overwrite an existing CMakeLists.txt.
 * @return Exit code of the CMake build command.
 */
int cmake_build(const Config& cfg, const ProjectLayout& pl,
                bool overwrite = false);

/**
 * Writes a CMakeLists.txt file for the given project configuration and layout.
 *
 * Generates target definitions, include paths, compiler settings, and
 * inter-target dependencies. If a CMakeLists.txt already exists and overwrite
 * is false, throws std::runtime_error. Also throws on I/O errors.
 *
 * @param cfg        Project configuration.
 * @param pl         Discovered project layout.
 * @param overwrite  Whether to overwrite an existing CMakeLists.txt.
 */
void write_cmakelists(const Config& cfg, const ProjectLayout& pl,
                      bool overwrite = false);

}  // namespace daemonmake

#endif