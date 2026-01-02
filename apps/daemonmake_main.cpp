#include <iostream>

#include "daemonmake/commands.hpp"

int main(int argc, char** argv) {
    using namespace daemonmake;

    if (argc < 2) {
        std::cerr << "Usage: daemonmake <command> [root]\n";
        return 1;
    }

    std::string cmd { argv[1] };
    std::string root { (argc >= 3) ? argv[2] : std::string{} };

    if (cmd == "init")     return run_init(root);
    if (cmd == "status")   return run_status(root);
    if (cmd == "build")    return run_build(root);
    if (cmd == "gencmake") return run_generate_cmake(root);
    if (cmd == "daemon")   return run_daemon(root);

    std::cerr << "Unknown command: " << cmd << std::endl;
    return 1;
}
