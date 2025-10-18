#pragma once

namespace istudio::generated {

template <typename T>
struct Pair {
  T first;
  T second;
};

template <typename T>
Pair<T> make_pair(T first, T second);

template <typename T>
Pair<T> swap(Pair<T> input);

}  // namespace istudio::generated
