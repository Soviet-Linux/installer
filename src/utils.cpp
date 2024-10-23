#include "../include/utils.hpp"

namespace fs = std::filesystem;

void copyFiles(const fs::path& source, const fs::path& destination) {
    try {
        if (fs::exists(source) && fs::is_directory(source)) {
            fs::create_directories(destination);

            for (const auto& entry : fs::directory_iterator(source)) {
                const auto& path = entry.path();
                auto destPath = destination / path.filename();

                if (fs::is_directory(path)) {
                    fs::copy(path, destPath, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
                } else {
                    fs::copy(path, destPath, fs::copy_options::overwrite_existing);
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    } catch (const fs::filesystem_error& e) {
    } catch (const std::exception& e) {
    }
}

std::vector<std::string> get_drives() {
    std::ifstream file("/proc/partitions");
    std::vector<std::string> drives;
    std::string line;

    if (!file.is_open()) {
        std::cerr << "Error: Could not open /proc/partitions" << std::endl;
        return drives;
    }

    std::getline(file, line);
    std::getline(file, line);

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        int major, minor;
        long long blocks;
        std::string name;

        if (iss >> major >> minor >> blocks >> name && minor == 0) {
            drives.push_back(name);
        }
    }

    return drives;
}

void deleteFilesInDir(const std::string& dir_path) {
    try {
        fs::path path(dir_path);

        if (fs::exists(path) && fs::is_directory(path)) {
            for (const auto& entry : fs::directory_iterator(path)) {
                if (fs::is_regular_file(entry.status())) {
                    fs::remove(entry.path());
                }
            }
            std::cout << "All files removed from " << dir_path << std::endl;
        } else {
            std::cerr << "Directory does not exist: " << dir_path << std::endl;
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

std::vector<std::string> readLocaleGenFile(const std::string& filePath) {
    std::ifstream file(filePath);
    std::vector<std::string> lines;
    std::string line;

    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filePath << std::endl;
        return lines;
    }

    while (std::getline(file, line)) {
        if (!line.empty() && line[0] == '#' && line.length() > 1 && line[1] != ' ') {
            lines.push_back(line);
        }
    }

    file.close();
    return lines;
}

std::vector<std::string> getKeymaps() {
    std::vector<std::string> keymaps;
    std::string command = "localectl list-keymaps";
    std::string result;

    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
    if (!pipe) {
        std::cerr << "Failed to open pipe." << std::endl;
        return keymaps;
    }

    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) {
        result += buffer;
    }

    std::istringstream iss(result);
    std::string line;
    while (std::getline(iss, line)) {
        if (!line.empty()) {
            keymaps.push_back(line);
        }
    }

    return keymaps;
}
