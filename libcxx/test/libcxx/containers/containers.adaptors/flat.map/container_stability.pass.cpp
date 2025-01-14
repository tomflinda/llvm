//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++03, c++11, c++14, c++17, c++20

// <flat_map>

// flat_map(key_container_type key_cont, mapped_container_type mapped_cont);
//
// libc++ uses stable_sort to ensure that flat_map's behavior matches map's,
// in terms of which duplicate items are kept.
// This tests a conforming extension.

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <flat_map>
#include <random>
#include <map>
#include <vector>

#include "test_macros.h"

struct Mod256 {
  bool operator()(int x, int y) const { return (x % 256) < (y % 256); }
};

int main(int, char**) {
  std::mt19937 randomness;
  std::vector<uint16_t> values;
  std::vector<std::pair<uint16_t, uint16_t>> pairs;
  for (int i = 0; i < 200; ++i) {
    uint16_t r = randomness();
    values.push_back(r);
    pairs.emplace_back(r, r);
  }

  {
    std::map<uint16_t, uint16_t, Mod256> m(pairs.begin(), pairs.end());
    std::flat_map<uint16_t, uint16_t, Mod256> fm(values, values);
    assert(fm.size() == m.size());
    LIBCPP_ASSERT(std::ranges::equal(fm, m));
  }
  {
    std::map<uint16_t, uint16_t, Mod256> m(pairs.begin(), pairs.end());
    std::flat_map<uint16_t, uint16_t, Mod256> fm(values, values, Mod256());
    assert(fm.size() == m.size());
    LIBCPP_ASSERT(std::ranges::equal(fm, m));
  }
  {
    std::map<uint16_t, uint16_t, Mod256> m(pairs.begin(), pairs.end());
    std::flat_map<uint16_t, uint16_t, Mod256> fm(values, values, std::allocator<int>());
    assert(fm.size() == m.size());
    LIBCPP_ASSERT(std::ranges::equal(fm, m));
  }
  {
    std::map<uint16_t, uint16_t, Mod256> m(pairs.begin(), pairs.end());
    std::flat_map<uint16_t, uint16_t, Mod256> fm(values, values, Mod256(), std::allocator<int>());
    assert(fm.size() == m.size());
    LIBCPP_ASSERT(std::ranges::equal(fm, m));
  }
  return 0;
}
