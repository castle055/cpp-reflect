// Copyright (c) 2024-2025, Víctor Castillo Agüero.
// SPDX-License-Identifier: GPL-3.0-or-later

// #include <cassert>

import reflect;
#include "gtest/gtest.h"

import std;

import packtl;
import reflect;

void setup() {
}

struct test_struct {
  std::string str1;
  std::string str2;
};

struct test_struct1 {
  const std::vector<test_struct>* vec { };
};

TEST(Equality, LensesEtAll) {
  test_struct1 ts1 = { };
  test_struct1 ts2 = { };

  bool eq = refl::deep_eq(ts1, ts2);
}
