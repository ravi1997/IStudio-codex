#include "genericpair.hpp"

namespace istudio::generated {

template <typename T>
Pair<T> make_pair(T first, T second) {
  return Pair<T>{first, second};
}

template <typename T>
Pair<T> swap(Pair<T> input) {
  return Pair<T>{input.second, input.first};
}

// Keep template definitions available to translation units that include this TU.
template Pair<int> make_pair<int>(int first, int second);
template Pair<int> swap<int>(Pair<int> input);

}  // namespace istudio::generated
