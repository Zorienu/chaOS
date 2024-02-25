#pragma once

namespace Math {

/*
 * Get the absolute value of the given number
 */
template<typename T>
T abs(T number) {
  return number >= 0 ? number : number * -1;
}

/*
 * Ceiling division to avoid truncating towards 0.
 * Examples:
 * - 0 / 512 = 0
 * - 1 / 512 = 1
 * - 511 / 512 = 1
 * - 512 / 512 = 1
 * - 513 / 512 = 2
 */
template<typename T, typename U>
T ceilDiv(T a, U b) {
  T result = a / b;

  if ((result % b) != 0) result++;

  return result;
}

}

