#ifndef UTILS_HPP
#define UTILS_HPP

#include <filesystem>
#include <thread>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;

void copyFiles(const fs::path& source, const fs::path& destination);

std::vector<std::string> get_drives();

void deleteFilesInDir(const std::string& dir_path);

std::vector<std::string> readLocaleGenFile(const std::string& filePath);

std::vector<std::string> getKeymaps();

#endif
