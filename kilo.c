// Imports
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

// Structures
struct termios orig_termios;
struct editorConfig
{
    int screenrows;
    int screencols;
    struct termios orig_termios;
};
struct editorConfig E;

// Init editor
void initEditor()
{
    if (getWindowSize(&E.screenrows, &E.screencols) == -1)
        die("getWindowSize");
}

// Enable raw mode and turn off echo
void enableRawMode()
{
    if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1)
        die("tcgetattr");

    atexit(disableRawMode);
    struct termios raw = E.orig_termios;
    // Turn off terminal emulators
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_cflag |= (CS8);
    // Fic CTRL+M
    raw.c_iflag &= ~(ICRNL | IXON);
    // Disable CTRL+S and CTRL+Q
    raw.c_iflag &= ~(IXON);
    // Turn off all processing
    raw.c_oflag &= ~(OPOST);
    // local flag and disable CTRL-V
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    // A timeout for read
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        die("tcsetattr");
}

// Disable raw mode and turn off echo
void disableRawMode()
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
}

// Error Handling
void die(const char *s)
{
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);

    perror(s);
    exit(1);
}

// getWindowSize
int getWindowSize(int *rows, int *cols)
{
    struct winsize ws;
    if (1 || ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
    {
        if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12)
            return -1;
        editorReadKey();
        return -1;
    }
    else
    {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}

// Refactor keyboard read
char editorReadKey()
{
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1)
    {
        if (nread == -1 && errno != EAGAIN)
            die("read");
    }
    return c;
}

// Refactor keyboard input
void editorProcessKeypress()
{
    char c = editorReadKey();
    switch (c)
    {
    case CTRL_KEY('q'):
        write(STDOUT_FILENO, "\x1b[2J", 4);
        write(STDOUT_FILENO, "\x1b[H", 3);
        exit(0);
        break;
    }
}

// Tidles
void editorDrawRows()
{
    int y;
    for (y = 0; y < E.screenrows; y++)
    {
        {
            write(STDOUT_FILENO, "~\r\n", 3);
        }
    }

    // Refresh the screen
    void editorRefreshScreen()
    {
        write(STDOUT_FILENO, "\x1b[2J", 4);
        write(STDOUT_FILENO, "\x1b[H", 3);

        editorDrawRows();
        write(STDOUT_FILENO, "\x1b[H", 3);
    }

    int main()
    {
        enableRawMode();
        initEditor();
        while (1)
        {
            editorRefreshScreen();
            editorProcessKeypress();
        }
        return 0;
    }