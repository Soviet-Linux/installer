#include "../include/ui.hpp"

std::string pickString(const std::vector<std::string>& options, const std::string& prompt) {
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    int height = 12;
    int width = 40;
    int startY = (LINES - height) / 2;
    int startX = (COLS - width) / 2;
    WINDOW* win = newwin(height, width, startY, startX);
    box(win, 0, 0);
    refresh();
    mvwprintw(win, 0, 1, prompt.c_str());
    int currentChoice = 0;
    int totalOptions = options.size();
    int scrollOffset = 0;

    while (true) {
        werase(win);
        box(win, 0, 0);
        mvwprintw(win, 0, 1, prompt.c_str());

        for (int i = 0; i < height - 3; ++i) {
            if (i + scrollOffset >= totalOptions) break;
            if (i + scrollOffset == currentChoice) {
                wattron(win, A_REVERSE);
                mvwprintw(win, i + 1, 1, options[i + scrollOffset].c_str());
                wattroff(win, A_REVERSE);
            } else {
                mvwprintw(win, i + 1, 1, options[i + scrollOffset].c_str());
            }
        }

        wrefresh(win);
        int ch = getch();
        switch (ch) {
            case KEY_UP:
                if (currentChoice > 0) {
                    --currentChoice;
                }
                if (currentChoice < scrollOffset) {
                    --scrollOffset;
                }
                break;
            case KEY_DOWN:
                if (currentChoice < totalOptions - 1) {
                    ++currentChoice;
                }
                if (currentChoice >= scrollOffset + (height - 3)) {
                    ++scrollOffset;
                }
                break;
            case KEY_ENTER:
            case 10:
                delwin(win);
                endwin();
                return options[currentChoice];
            case 27:
                delwin(win);
                endwin();
                return "";
        }
    }

    delwin(win);
    endwin();
    return "";
}

std::string displayInputBox(const std::string& prompt) {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    int height = 5;
    int width = prompt.length() + 4;
    int startY = (LINES - height) / 2;
    int startX = (COLS - width) / 2;
    WINDOW* inputWin = newwin(height, width, startY, startX);
    box(inputWin, 0, 0);
    mvwprintw(inputWin, 1, 2, "%s", prompt.c_str());
    mvwprintw(inputWin, 3, 2, "Input: ");
    wrefresh(inputWin);
    char input[256] = "";
    echo();
    mvwgetnstr(inputWin, 3, 9, input, sizeof(input) - 1);
    delwin(inputWin);
    endwin();
    return std::string(input);
}
