// Copyright (c) 2024-2025, Víctor Castillo Agüero.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gtest/gtest.h"
#include "test_types.h"

import std;
import refl.generation.Reflector;

using namespace refl;


template<typename T>
using fun_type = T(int, ...) const noexcept;

TEST(FunctionInfo, SameInstanceReturned) {      //
  const auto &info = reflect<fun_type<char>>(); //.as<TypeInfo::FUNCTION>();
  std::println("Function Name: {}", info.name);
}
