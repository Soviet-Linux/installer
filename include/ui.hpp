#include <vector>
#include <string>
#include <ncurses.h>
#include <cstring>

std::string pickString(const std::vector<std::string>& options, const std::string& prompt);

std::string displayInputBox(const std::string& prompt);
