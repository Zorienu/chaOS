/*
 * Get the size in bytes of the given string
 *
 * string: the string to calculate the length
 *
 * returns: the size of the string
 */
unsigned long strlen (char *string) {
  int result = 0;

  while(*string++) result++; 

  return result;
}
