#include "../include/utils.hpp"
#include "../include/ui.hpp"
#include "../include/install.hpp"
#include <ncurses.h>
#include <iostream>

int main() {
    std::vector<std::string> drives = get_drives();

    std::vector<std::string> locales = getLocales();
    std::vector<std::string> keymaps = getKeymaps();

    std::string drive = pickString(drives, "Select a drive:");
    clear();
    refresh();

    std::string locale = pickString(locales, "Select a locale:");
    clear();
    refresh();

    std::string keymap = pickString(keymaps, "Select a keymap:");
    clear();
    refresh();

    std::string rootPassword = displayInputBox("Please enter the root password:");
    clear();
    refresh();

    std::string hostname = displayInputBox("Please enter the hostname:");
    clear();
    refresh();

    endwin();
    // TODO: i have no idea why this prints so weirdly
    std::cout << "Drive: " << drive << std::endl;
    std::cout << "Locale: " << locale << std::endl;
    std::cout << "Keymap: " << keymap << std::endl;
    std::cout << "Root Password: " << rootPassword << std::endl;
    std::cout << "Hostname: " << hostname << std::endl;

    std::cout << "Are you sure you want to continue (y/n)? ";
    char confirmation;
    std::cin >> confirmation;

    if (confirmation == 'y' || confirmation == 'Y') {
        std::cout << "Continuing with the setup..." << std::endl;
        install_soviet(drive, locale, keymap, rootPassword, hostname);
    } else {
        std::cout << "Setup canceled." << std::endl;
    }

    return 0;
}
