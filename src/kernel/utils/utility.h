#pragma once

/*
 * Implementation of std::move
 * This does not move anything in reality
 * This just converts the given value from al l-value to an r-value reference
 * to tell the destination object to use the move-semantics instead of the copy semantics
 */
template<typename T>
T&& move(T& value) {
  return (T&&)value;
}
