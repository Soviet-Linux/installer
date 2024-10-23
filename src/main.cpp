#include <cstdlib>
#include <string>
#include <sys/mount.h>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <cerrno>
#include <vector>
#include <ncurses.h>
#include <fstream>
#include <sstream>

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

void display_menu(WINDOW *menu_win, const std::vector<std::string>& drives, int highlight) {
    int x = 2, y = 1;
    box(menu_win, 0, 0);
    for (int i = 0; i < drives.size(); ++i) {
        if (highlight == i + 1) {
            wattron(menu_win, A_REVERSE);
            mvwprintw(menu_win, y, x, "%s", drives[i].c_str());
            wattroff(menu_win, A_REVERSE);
        } else {
            mvwprintw(menu_win, y, x, "%s", drives[i].c_str());
        }
        ++y;
    }
    wrefresh(menu_win);
}

bool confirm_selection(const std::string& drive) {
    int x = 4, y = 6;
    clear();
    mvprintw(y, x, "You have selected: %s", drive.c_str());
    mvprintw(y + 2, x, "Confirm selection? (y/n)");
    refresh();

    while (true) {
        int c = getch();
        if (c == 'y' || c == 'Y') {
            return true;
        } else if (c == 'n' || c == 'N') {
            return false;
        }
    }
}

void display_warning_dialog() {
    clear();
    mvprintw(1, 1, "Welcome to the Soviet Linux Installation");
    mvprintw(2, 1, "The installation should be pretty straight forward,");
    mvprintw(3, 1, "if you are in trouble ask for help in our Discord.");
    mvprintw(5, 1, "Press any key to continue...");
    refresh();
    getch();
    clear();
}

int mount_root() {
    const char* source = "LABEL=sovietlinux";
    const char* target = "/mnt";
    const char* filesystemtype = "btrfs";
    const char* options = "compress=zstd:3";

    if (mount(source, target, filesystemtype, 0, options) != 0) {
        std::cerr << "Failed to mount: " << std::strerror(errno) << std::endl;
        return -1;
    }

    std::cout << "Successfully mounted " << source << " at " << target
              << " with options: " << options << std::endl;

    return 0;
}

int mount_boot() {
    const char* source = "LABEL=SOVIET-EFI";
    const char* target = "/mnt/efi";
    const char* filesystemtype = "vfat";
    const char* options = "defaults";

    if (mount(source, target, filesystemtype, 0, options) != 0) {
        std::cerr << "Failed to mount: " << std::strerror(errno) << std::endl;
        return -1;
    }

    std::cout << "Successfully mounted " << source << " at " << target
              << " with options: " << options << std::endl;

    return 0;
}

int partition_drive(const std::string& target_drive) {
    return std::system(("sgdisk -n 1:0:+512M -c 1:\"SOVIET-EFI\" -t 1:ef00 -n 2:0:0 -c 2:\"sovietlinux\" -t 2:8304 " + target_drive).c_str());
}

int create_boot(const std::string& target_drive) {
    return std::system(("mkfs.vfat -F 32 -n SOVIET-EFI " + target_drive + "1").c_str());
}

int create_root(const std::string& target_drive) {
    return std::system(("mkfs.btrfs -L sovietlinux " + target_drive + "2").c_str());
}

void install_soviet(std::string target_drive) {
    partition_drive(target_drive);
    create_boot(target_drive);
    create_root(target_drive);
    mount_root();
    mount_boot();
}

int main() {
    initscr();
    clear();
    noecho();
    cbreak();
    curs_set(0);

    display_warning_dialog();

    int highlight = 1;
    int choice = 0;
    int c;

    std::vector<std::string> drives = get_drives();
    int n_drives = drives.size();

    WINDOW *menu_win = newwin(10, 40, 4, 4);
    keypad(menu_win, TRUE);

    mvprintw(2, 4, "Soviet Installer");
    mvprintw(3, 4, "Select the drive to install to:");
    refresh();

    while (true) {
        if (n_drives == 0) {
            mvprintw(6, 4, "No drives found.");
            refresh();
            getch();
            break;
        }

        display_menu(menu_win, drives, highlight);
        c = wgetch(menu_win);

        switch (c) {
            case KEY_UP:
                if (highlight == 1) {
                    highlight = n_drives;
                } else {
                    --highlight;
                }
                break;
            case KEY_DOWN:
                if (highlight == n_drives) {
                    highlight = 1;
                } else {
                    ++highlight;
                }
                break;
            case 10:
                choice = highlight;
                break;
            default:
                break;
        }

        if (choice != 0) {
            break;
        }
    }

    if (choice != 0) {
        if (confirm_selection(drives[choice - 1])) {
            clear();
            mvprintw(4, 4, "Drive confirmed: %s", drives[choice - 1].c_str());
            install_soviet(drives[choice - 1]);
            mount_root();
        } else {
            clear();
            mvprintw(4, 4, "Selection cancelled. Exiting...");
            refresh();
            getch();
            endwin();
            return 0;
        }
        refresh();
        getch();
    }

    endwin();
    return 0;
}
