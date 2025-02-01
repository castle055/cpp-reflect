
// Copyright (c) 2024-2025, Víctor Castillo Agüero.
// SPDX-License-Identifier: GPL-3.0-or-later

// #include <cassert>
#include "common.h"

import reflect;

import packtl;
import reflect.serialize;

void setup() {
}

template <typename T>
struct a {
  int aa = 1234;

  void asdf() {}

private:
  T wow;
};

struct bb {
  a<int>    a1{};
  a<double> a2{};
};

struct c {
  // bb& baby;
  struct test {
    int b;
  } props;
  int as;

  void hello(int a) {}

private:
  void hello(std::string a, int v) {}
};

namespace charts {}

TEST("Nominal Start-up") {
  std::cout << "STARTING TEST" << std::endl;
  //    a::field_names;

  a<int> obj{};
  a<int> obj1{};
  // typename a<int>::__type_info__ fdsa{};

  // print_obj(obj);

  // auto str = refl::serializer<a<int>, refl::formats::reflected>::to_string(obj);

  // std::cout << str << std::endl;

  bb b{};
  refl::serializer<>::to_stream(std::cout, c{});

  // bool eq = obj == obj1;
  // if (eq) {
  //   std::cout << "OBJECTS ARE EQUAL!" << std::endl;
  // } else {
  //   std::cout << "OBJECTS ARE NOT EQUAL!" << std::endl;
  // }

  // obj.name = "General kenobi...";

  // eq = obj == obj1;
  // if (eq) {
  //   std::cout << "OBJECTS ARE EQUAL!" << std::endl;
  // } else {
  //   std::cout << "OBJECTS ARE NOT EQUAL!" << std::endl;
  // }

  return 0;
}

struct hola {
  int a = 4;
};

struct rt_test {
  int  a = 1;
  int  b = 2;
  int  c = 3;
  // std::string b;
  hola h;
};

TEST("Runtime Reflection") {
  auto rti = refl::type_info::from<rt_test>();

  rt_test test;
  for (const auto& field: rti.fields()) {
    void* f_ptr = field.get_ptr(&test);
    if (field.type().is_type<hola>()) {
      int& f_ref = field.get_ref<int>(&test);

      // TODO
      // LOG::print{INFO
      // }("Field: {}: [{}] [{}] = (0x{:X}) {};",
      //   field.offset,
      //   field.type().name(),
      //   field.name,
      //   (std::size_t)f_ptr,
      //   f_ref);
    }
  }

  return 0;
}

struct [[refl::annotation]] ignore {};

struct [[refl::annotation]] json_name {
  // consteval explicit json_name(std::string&& name_): name(name_) {}
  std::string name;
};

template <typename F>
struct [[refl::annotation]] lambda {
  // consteval explicit json_name(std::string&& name_): name(name_) {}
  F func;
  // int n;
};

struct annotate_me {
  [[meta(ignore{})]]
  int a = 5;

  [[meta("asdf")]] [[meta(json_name{"hello"})]] [[meta(lambda{[](int i) { return 2 * i + 123; }})]]
  std::string str = "hello there!";

  [[refl::ignore]]
  std::tuple<const char*> tt = {"asdf"};
};

TEST("Annotations") {
  annotate_me am{};

  refl::serializer<>::to_stream(std::cout, am);
  refl::serializer<
    formats::json_fmt>::to_stream(std::cout, refl::field<annotate_me, 1>::metadata_item<1>);
  std::cout << std::format("{}", refl::field<annotate_me, 1>::metadata_item<2>.func(2))
            << std::endl;

  constexpr std::tuple<const char*> t = {"asdf"};
  return 0;
}
