#include "client.h"

void init_bordered_window(BorderedWindow *window, int x, int y, int width, int height)
{
    window->border_window = newwin(height, width, y, x);
    box(window->border_window, 0, 0);
    wrefresh(window->border_window);
    window->window = newwin(height-2, width-2, y+1, x+1);
    scrollok(window->window, true);
    wrefresh(window->window);
}

void initialize_ncurses()
{
    initscr();
    refresh();
    init_bordered_window(&message_window, 0, 0, COLS, LINES-5);
    init_bordered_window(&input_window, 0, LINES-5, COLS, 5);
}

void handle_resize(char *message)
{
    endwin();
    initialize_ncurses();
    // TODO: print old messages
    wprintw(input_window.window, message);
}

void *TUI_main(void *vargp)
{
    initialize_ncurses();
    char message[1024] = {0};
    int message_position = 0;
    while (1)
    {
        char c = wgetch(input_window.window);
        if (c == '\b' || c == 127)
        {
            wprintw(input_window.window, "\b\b  \b\b");
            if (message_position != 0)
            {
                wprintw(input_window.window, "\b  \b\b");
                message[message_position--] = 0;
            }
            wrefresh(input_window.window);
        } else if (c == KEY_RESIZE || c == -102)
        {
            handle_resize(message);
        } else
        {
            message[message_position++] = c;
        }
        if (c != '\n')
        {
            continue;
        }
        message[message_position] = 0;    
        werase(input_window.window);
        wprintw(message_window.window, "you: %s", message);
        wrefresh(message_window.window);
        sendMessage(message);
        message_position = 0;
    }
    return NULL;
}

