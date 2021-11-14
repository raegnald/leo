#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

// Original terminal state
struct termios original_termios;


void restoreTerminalState() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
}

void enableRawMode() {
  // Read the current terminal attributes into a struct
  tcgetattr(STDIN_FILENO, &original_termios);
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


  // Update the terminal with the changes
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}


int main () {
  // Prepare the terminal to enter raw mode from canonical mode
  enableRawMode();

  char c;

  while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q') {
    // Check if it a control char in order to display it differently
    if (iscntrl(c)) {
      printf("%05d\r\n", c);
    } else {
      printf("%05d '%c'\r\n", c, c);
    }
  }

  return 0;
}
