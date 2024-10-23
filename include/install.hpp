#include <iostream>
#include <cerrno>
#include <ostream>
#include <sys/mount.h>
#include <cstring>

void install_soviet(const std::string& target_drive,
                    const std::string& locale,
                    const std::string& keymap,
                    const std::string& rootPassword,
                    const std::string& hostname);
