// Copyright (c) 2024-2025, Víctor Castillo Agüero.
// SPDX-License-Identifier: GPL-3.0-or-later

// #include <cassert>

import reflect;
#include "gtest/gtest.h"

import std;

import packtl;
import reflect.deserialize;
import reflect.serialize;
// import reflect.optics;

void setup() {
}

struct serialize_me {
  int a              = 5;
  std::string str    = "hello there!";
  serialize_me* next = nullptr;
};

TEST(Deserialization, Deserialize) {
  // static_assert(refl::Reflected<serialize_me>);
  // static_assert(refl::Reflected<template_test<serialize_me>>);
  // static_assert(refl::Reflected<template_test<serialize_me>::inner2>);
  // static_assert(refl::Reflected<template_test<serialize_me>::inner2::inner>);
  // serialize_me sm1{};
  // serialize_me sm{};
  // sm.next = &sm1;

  refl::JSON some{};
  some["a"] = 321;
  auto sm1 = refl::from_json<serialize_me>(some);
  std::cout << refl::to_json(sm1) << std::endl;
  auto sm = refl::from_json<serialize_me>(R"json({"a":123,"str":"Holy shit! this seems to work (for now)"})json");
  std::cout << refl::to_json(sm) << std::endl;
}
