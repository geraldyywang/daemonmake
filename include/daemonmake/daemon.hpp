#ifndef DAEMONMAKE__DAEMONMAKE_DAEMON
#define DAEMONMAKE__DAEMONMAKE_DAEMON

#include "daemonmake/config.hpp"
#include "daemonmake/project.hpp"

#include <filesystem>
#include <vector>
#include <map>

namespace daemonmake {

class Daemon {
public:
    explicit Daemon(const Config& cfg);
    int run();
    
private:
    void snapshot_timestamps();
    std::vector<std::filesystem::path> get_changed_files();
    int rebuild_all();
    int rebuild_changed();

    Config cfg_;
    ProjectLayout pl_;
    std::map<std::filesystem::path, std::filesystem::file_time_type> last_timestamps_;
};

}  //namespace daemonmake

#endif