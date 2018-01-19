/*
 *  atoi.s
 *  Converts a string to an int.
 *
 *  Inputs: Pointer to string
 *  Returns: Int
 *  Modifies:
 *
 *  int atoi(char *str) {
 *      const char *start = str;
 *      do {
 *          register char c = IN();
 *          *str = c;
 *          ++str;
 *      } while(c != '\n');
 *
 *      return (int) (str - start);
 *  }
 *
 *  Bytecode: x bytes.



 */

int atoi() {
