#ifndef DAEMONMAKE__DAEMONMAKE_COMMANDS
#define DAEMONMAKE__DAEMONMAKE_COMMANDS

#include <string>

namespace daemonmake {

int run_init(const std::string& root_arg);

int run_status(const std::string& root_arg);

int run_build(const std::string& root_arg);

int run_daemon(const std::string& root_arg);

}  // namespace daemonmake

#endif