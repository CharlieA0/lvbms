
#include "repl.h"
#include "uart.h"

#include <stdbool.h>
#include <string.h>
#include <ctype.h>

static void clear_state(char argv[MAX_ARGS][MAX_ARG_LEN], uint8_t* argc, uint8_t* char_count);

/**
 * @brief prints the string msg with the output prompt
 *
 * @param msg string to print
 */
void output(char* msg) {
  
  // print output 
  print("\n\rout> ");
  print(msg);

}


/**
 * @brief Cleans up after a command has been completed. Prints prompt for new input.
 *
 * @note This is called by repl wrapper, not in passed function.
 *
 * @param argv[MAX_ARGS][MAX_ARG_LEN]   points to arguement array
 * @param argc                          pointer to arguement count
 * @param char_count                    pointer to character count
 */
static void clear_state(char argv[MAX_ARGS][MAX_ARG_LEN], uint8_t* argc, uint8_t* char_count) {

  // clear command state
  *argc = 0;
  *char_count = 0;

  for (int i = 0; i < MAX_ARGS; i++) {
    argv[i][0] = '\0';
  }
  
  // print prompt
  print("\n\rinp> ");

}


/**
 * @brief Checks if two strings are equal. It can be used to match arguments quickly.
 *
 * @param str1  a string
 * @param str2  another string
 *
 * @return boolean true iff every character in str1 == every character in str2
 */
bool eq(char* str1, char* str2) {
  return (strcmp(str1, str2) == 0);
}



/**
 * @brief Call this to begin repl loop. 
 *
 * @param commandHandler  Called when new commands are entered. It's passed an argument count argc and
 *                        a argument vector argv whose first value is the name of called command. 
 *                        This should return a REPL_Status, REPL_CONTINUE or REPL_EXIT.
 *
 * @return    returns final REPL_Status 
 */
REPL_Status repl(REPL_Status (* commandHandler) (int, char[MAX_ARGS][MAX_ARG_LEN])) {

  // REPL State
  REPL_Status status = REPL_CONTINUE;
  
  char argv[MAX_ARGS][MAX_ARG_LEN];
  argv[0][0] = '\0';
  uint8_t argc = 0;
  uint8_t char_count = 0;
  
  bool first_input = true;
  
  clear_state(argv, &argc, &char_count);

  // Serial State
  HAL_StatusTypeDef serial_status;
  char c;

  do {

    // Get serial input
    serial_status = input(&c, sizeof(c));

    // If there's input
    if (serial_status == HAL_OK) {

      // Print the prompt first time
      if (first_input) {
        clear_state(argv, &argc, &char_count);
        first_input = false;
        continue;
      }

      // process command on enter -- this is carriage return for some reason
      if (c == '\r') {

        char* cmd = argv[0];

        // swallow empty enters
        if (eq(cmd, "")) {
          clear_state(argv, &argc, &char_count);
          continue;
        }

        // Built-in commands
        if (eq(cmd, "echo")) {

          // Example for later
          output("Unimplemented");
          clear_state(argv, &argc, &char_count);
          continue;
        }

        // User commands
        status = commandHandler(argc, argv);
        clear_state(argv, &argc, &char_count);

        // User outputs a REPL_Status
        // We could handle errors and such here...

        // E.g. Catch unknown commands
        // output("Unknown Command");
        // clear_state(argv, &argc, &char_count);
        
        continue;
      }

      // abort on backtick
      if (c == '`') {
        print_char(&c);
        output("ABORT"); 
        clear_state(argv, &argc, &char_count);
        continue;
      }

      // handle spaces
      else if (isspace(c)) {

        // Swallow leading spaces
        if (char_count == 0) {
          print_char(&c);
          continue;
        }

        // Catch arguement count overflow
        if (argc >= MAX_ARGS) {
          output("TOO MANY ARGUEMENTS");
          clear_state(argv, &argc, &char_count); 
          continue;
        }

        // Otherwise tokenize arguements
        argc++;
        char_count = 0;
        continue;
      }
     
      // Catch arguement length overflow
      if (char_count >= MAX_ARG_LEN) {
        output("ARGUEMENT TOO LONG");
        clear_state(argv, &argc, &char_count);
        continue;
      }

      // print the character
      print_char(&c);

      // Store the character
      argv[argc][char_count] = c;
      argv[argc][char_count + 1] = '\0';
      char_count++; 

    }

  } while (status == REPL_CONTINUE);

  return status;

}
