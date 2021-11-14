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

  // Turn off the `ECHO` feature (when activated lets you see what you type)
  // and also `ICANON`, so that we enter raw mode (read the input by bytes, not
  // by lines)
  raw.c_lflag &= ~(ECHO | ICANON);

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
      printf("%05d\n", c);
    } else {
      printf("%05d '%c'\n", c, c);
    }
  }

  return 0;
}
