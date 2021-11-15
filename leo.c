#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

#define CTRL_KEY(key) ((key) & 0x1f)

// Original terminal state
struct termios original_termios;



void failwith(const char *s) {
  perror(s);
  exit(1);
}


void restoreTerminalState() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios) == -1)
      failwith("tcsetattr on restoring original state");
}

void enableRawMode() {
  // Read the current (aka. the original) terminal attributes into a struct
  if (tcgetattr(STDIN_FILENO, &original_termios) == -1)
      failwith("tcgetattr on reading original state");

  atexit(restoreTerminalState);

  // Container of terminal data that we are going to change
  struct termios raw = original_termios;

  // Turn off the following:
  //     - `ECHO` (when activated lets you see what you type).
  //     - `ICANON`, so that we enter raw mode (read the input by bytes, not by
  // lines).
  //     - `ISIG`, so that Ctrl-C and Ctrl-Z do not send SIGINT nor SIGTSTP,
  // respectively.
  //     - `IXON`, so that Ctrl-S and Ctrl-Q do not work (they block/resume the
  // input flow.
  //     - `IEXTEN`, some machines take another characther when you type
  // Ctrl-V, and we don't want that, so we disable it.
  //     - `ICRNL`, so that Ctrl-M return 13, not 10.
  //     - `OPOST`, because we want new lines to be "\n", not "\r\n".

  raw.c_lflag &= ~( ECHO   | 
                    ICANON | 
                    ISIG   |
                    IEXTEN );

  raw.c_iflag &= ~( IXON   |
                    ICRNL  );

  raw.c_oflag &= ~( OPOST  );

  // VMIN: min number of bytes for `read()` to return
  // VTIME: max amount of time to wait before `read()` returns
  raw.c_cc[VMIN]  = 0;
  raw.c_cc[VTIME] = 1; // measure: 1/10 second

  // Update the terminal with the changes
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
      failwith("tcsetattr on updating terminal state");
}



char editorReadKey() {
  int readResult; // if -1: error
  char c;
  // Do not return a character unless you reveive a keypress
  while ((readResult = read(STDIN_FILENO, &c, 1)) != 1) {
    if (readResult == -1 && errno != EAGAIN)
        failwith("cannot read key");
  }
  return c;
}



void editorRefreshScreen() {
  // Using standard VT100 escape sequences, they all start with:
  //    \x1b[ : escape


  //    2J    : J command (Erase in display, aka clear screen)
  //            argument 2 (clear the whole screen, not just upto the cursor)
  write(STDOUT_FILENO, "\x1b[2J", 4);
  
  // H command (cursor position), with no arguments it puts the cursor on the
  // first line, first column
  write(STDOUT_FILENO, "\x1b[H", 3);
}



void editorProcessKeypress() {
  // Wait for a keypress (or an escape sequence--TODO)
  char c = editorReadKey();
  // Handle that keypress correspondingly
  switch (c) {
    case CTRL_KEY('q'):
      exit(0);
      break;
  }
}



int main () {
  enableRawMode();

  while (1) {
    editorRefreshScreen();
    editorProcessKeypress();
  }

  return 0;
}

