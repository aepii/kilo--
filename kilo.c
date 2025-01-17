/*** includes ***/

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

/*** data ***/

// Hold the original terminal settings to restore them after disabling raw mode
struct termios original_termios; 

/*** terminal ***/

// Descriptive error handling
void die(const char *s) {
  perror(s);
  exit(1);
}

// Disable raw mode
void disableRawMode() {
  // Set the original terminal settings
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios) == -1) {
    die("tcsetattr"); 
  }
}

// Enable raw mode
void enableRawMode() {
  // Save the original terminal settings
  if (tcgetattr(STDIN_FILENO, &original_termios) == -1) {
    die("tcgetattr");
  } 
  atexit(disableRawMode); // Disable raw mode on termination

  struct termios raw = original_termios; // Create a copy of original terminal settings to raw

  /*
  Disable signal interruption on break, carriage return to newline mapping, 
  input parity check, strip character, and output control
  */ 
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON); 

  raw.c_oflag &= ~(OPOST); // Disable output processing
  raw.c_cflag |= CS8; // Set character size to 8 bits per byte
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG); // Disable echo, canonical input, extended input character processing, and signals

  raw.c_cc[VMIN] = 0; // Set the minimum number of bytes of input to 0 needed before read() can return
  raw.c_cc[VTIME] = 1; // Set the maximum amount of time to 100ms before read() can return
  
  // Apply the modified settings to the terminal
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
    die("tcsetattr");
  }
}

/*** init ***/

int main() {
  enableRawMode();

  while (1) {
    char c = '\0';
    if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN)  {
      die("read");
    }
    // Check if the byte is a control character
    if (iscntrl(c)) { 
      printf("%d\r\n", c); // Display the byte as a decimal number
    } else {
      printf("%d ('%c')\r\n", c, c); // Display the byte as a decimal number and a character
    }
    if (c == 'q') break;
  }

  return 0;
}