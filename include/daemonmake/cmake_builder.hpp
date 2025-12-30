#ifndef DAEMONMAKE__DAEMONMAKE_CMAKE_BUILDER
#define DAEMONMAKE__DAEMONMAKE_CMAKE_BUILDER

#include "daemonmake/config.hpp"
#include "daemonmake/project.hpp"

namespace daemonmake {

int cmake_build(const Config& cfg);

/**
 * Generate a CMakeLists.txt for the given project.
 * This inspects the discovered targets in `ProjectLayout` and emits a
 * minimal, modern CMake file that:
 * - sets the project name and C++ standard,
 * - defines one `add_library` / `add_executable` per target,
 * - wires up `target_link_libraries` according to inferred dependencies.
 * 
 * The CMakeLists.txt is written to `<cfg.project_root>/CMakeLists.txt`.
 * By default, this function refuses to overwrite an existing file unless
 * `overwrite` is set to true. This is to avoid clobbering any hand-written
 * CMake project that may already exist in the tree.
 * 
 * Throws `std::runtime_error` on I/O errors.
 */
void write_cmakelists(const Config& cfg, const ProjectLayout& pl, bool overwrite = false);

}  // namespace daemonmake

#endif