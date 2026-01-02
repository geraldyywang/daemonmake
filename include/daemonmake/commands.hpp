#ifndef DAEMONMAKE__DAEMONMAKE_COMMANDS
#define DAEMONMAKE__DAEMONMAKE_COMMANDS

#include <string>

namespace daemonmake {

/**
 * Initializes a daemonmake project under the given root.
 *
 * Creates a default config, saves it, scans the project for targets and
 * dependencies, and prints a summary. On error, logs to stderr.
 *
 * @param root_arg Project root path. If empty, uses the current directory.
 * @return 0 on success, non-zero on failure.
 */
int run_init(const std::string& root_arg);

/**
 * Prints the current project configuration and discovered targets.
 *
 * Loads the config from disk, rebuilds the project layout and dependency
 * graph, and prints a summary. Does not modify any files.
 *
 * @param root_arg Project root path. If empty, uses the current directory.
 * @return 0 on success, non-zero on failure.
 */
int run_status(const std::string& root_arg);

/**
 * Performs a one-shot build of the project using CMake.
 *
 * Loads the config, discovers targets, infers dependencies, and invokes
 * cmake_build() for the project. Exceptions are caught and mapped to a
 * non-zero return code.
 *
 * @param root_arg Project root path. If empty, uses the current directory.
 * @return Exit code from cmake_build() on success, 1 on error.
 */
int run_build(const std::string& root_arg);

/**
 * Generates a CMakeLists.txt for the project without building it.
 *
 * Loads the project configuration, discovers targets and dependencies,
 * and writes a CMakeLists.txt into the project root. Fails if a
 * CMakeLists.txt already exists unless overwriting is allowed by the
 * underlying writer.
 *
 * @param root_arg Project root path. If empty, uses the current directory.
 * @return 0 on success, non-zero on failure.
 */
int run_generate_cmake(const std::string& root_arg);

/**
 * Runs the daemon in the foreground for the given project.
 *
 * Loads the config, constructs a Daemon, and enters its main loop. This
 * call blocks until the daemon terminates or an exception is thrown.
 *
 * @param root_arg Project root path. If empty, uses the current directory.
 * @return 0 on clean exit, 1 if an exception is thrown.
 */
int run_daemon(const std::string& root_arg);


}  // namespace daemonmake

#endif