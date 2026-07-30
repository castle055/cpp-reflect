// Copyright (c) 2024-2025, Víctor Castillo Agüero.
// SPDX-License-Identifier: GPL-3.0-or-later

// #include <cassert>

#include "gtest/gtest.h"
import reflect;

import std;

import packtl;
import reflect.serialize;

void setup() {
}

struct serialize_me {
  int a              = 5;
  std::string str    = "hello there!";
  serialize_me* next = nullptr;
};

TEST(Serialization, Serialize) {
  serialize_me sm1 { };
  serialize_me sm { };
  sm.next = &sm1;

  refl::serializer<formats::json_fmt>::to_stream(std::cout, sm);

  std::cout << refl::to_json(sm);
}

template<template <typename> typename Format, typename T>
int check_serializes_to(const T &t, const std::string &serialized) {
  std::string str = refl::serializer<Format>::to_string(t);

  if (refl::field_count<T> >= 0) {
    [&]<std::size_t... I>(std::index_sequence<I...>) {
      ((std::cout << "FIELD TYPE: "
        << std::format("{:?}", refl::type_name<typename refl::field<T, I>::type>)
        << std::endl),
        ...);
    }(std::make_index_sequence<refl::field_count<T>> { });
  }
  std::cout << "FORMAT: " << std::format("{:?}", refl::pack_name<Format>) << std::endl;
  std::cout << "SERIALIZED: " << std::format("{:?}", str) << std::endl;
  if (str == serialized) {
    std::cout << " * OK" << std::endl;
    return 0;
  }
  std::cout << " * EXPECTED: " << std::format("{:?}", serialized) << std::endl;
  return 1;
}

TEST(Serialization, JSONEmpty) {
  struct test_struct {
  } test_obj { };
  check_serializes_to<formats::json_fmt>(test_obj, "{}");
}


template<typename Field>
struct test_one_field_struct {
  Field value;
};

template<typename Field, template <typename> typename Format>
int check_field_serialization(const std::string &serialized, Field default_value = { }) {
  test_one_field_struct<Field> test_obj {std::move(default_value)};
  return check_serializes_to<Format>(test_obj, serialized);
}

TEST(Serialization, JSONIntZero) {
  check_field_serialization<int, formats::json_fmt>("{\"value\":0}");
}

TEST(Serialization, JSONShortZero) {
  check_field_serialization<short, formats::json_fmt>("{\"value\":0}");
}

TEST(Serialization, JSONLongZero) {
  check_field_serialization<long, formats::json_fmt>("{\"value\":0}");
}

TEST(Serialization, JSONUnsignedIntZero) {
  check_field_serialization<unsigned int, formats::json_fmt>("{\"value\":0}");
}

TEST(Serialization, JSONUnsignedShortZero) {
  check_field_serialization<unsigned short, formats::json_fmt>("{\"value\":0}");
}

TEST(Serialization, JSONUnsignedLongZero) {
  check_field_serialization<unsigned long, formats::json_fmt>("{\"value\":0}");
}

TEST(Serialization, JSONFloatZero) {
  check_field_serialization<float, formats::json_fmt>("{\"value\":0.0}");
}

TEST(Serialization, JSONDoubleZero) {
  check_field_serialization<double, formats::json_fmt>("{\"value\":0.0}");
}

TEST(Serialization, JSONBoolTrue) {
  check_field_serialization<bool, formats::json_fmt>("{\"value\":\"true\"}", true);
}

TEST(Serialization, JSONBoolFalse) {
  check_field_serialization<bool, formats::json_fmt>("{\"value\":\"false\"}", false);
}

TEST(Serialization, JSONChar) {
  check_field_serialization<char, formats::json_fmt>("{\"value\":\"c\"}", 'c');
}

TEST(Serialization, JSONCharPointerString) {
  char str[] = "hello, world!";
  check_field_serialization<char*, formats::json_fmt>("{\"value\":\"hello, world!\"}", str);
}

TEST(Serialization, JSONConstCharPointerString) {
  check_field_serialization<const char*, formats::json_fmt>(
    "{\"value\":\"hello, world!\"}", "hello, world!"
  );
}

TEST(Serialization, JSONStdString) {
  check_field_serialization<std::string, formats::json_fmt>(
    "{\"value\":\"hello, world!\"}", "hello, world!"
  );
}

TEST(Serialization, JSONStdStringView) {
  check_field_serialization<std::string_view, formats::json_fmt>(
    "{\"value\":\"hello, world!\"}", "hello, world!"
  );
}

TEST(Serialization, JSONStdVector) {
  check_field_serialization<std::vector<int>, formats::json_fmt>(
    "{\"value\":[1,2,3,4]}", {1, 2, 3, 4}
  );
}

TEST(Serialization, JSONStdArray) {
  check_field_serialization<std::array<int, 4>, formats::json_fmt>(
    "{\"value\":[1,2,3,4]}", {1, 2, 3, 4}
  );
}

TEST(Serialization, JSONStdList) {
  check_field_serialization<std::list<int>, formats::json_fmt>(
    "{\"value\":[1,2,3,4]}", {1, 2, 3, 4}
  );
}

TEST(Serialization, JSONStdDeque) {
  check_field_serialization<std::deque<int>, formats::json_fmt>(
    "{\"value\":[1,2,3,4]}", {1, 2, 3, 4}
  );
}

TEST(Serialization, JSONStdIntMap) {
  check_field_serialization<std::map<int, int>, formats::json_fmt>(
    "{\"value\":[[1,1],[2,2],[3,3],[4,4]]}", {{1, 1}, {2, 2}, {3, 3}, {4, 4}}
  );
}

TEST(Serialization, JSONStdUnorderedIntMap) {
  check_field_serialization<std::unordered_map<int, int>, formats::json_fmt>(
           "{\"value\":[[1,1],[2,2],[3,3],[4,4]]}", {{1, 1}, {2, 2}, {3, 3}, {4, 4}}
         ) and check_field_serialization<std::unordered_map<int, int>, formats::json_fmt>(
           "{\"value\":[[4,4],[3,3],[2,2],[1,1]]}", {{1, 1}, {2, 2}, {3, 3}, {4, 4}}
         );
}

TEST(Serialization, JSONStdStringMap) {
  check_field_serialization<std::map<std::string, int>, formats::json_fmt>(
    "{\"value\":{\"A\":1,\"B\":2,\"C\":3,\"D\":4}}", {{"A", 1}, {"B", 2}, {"C", 3}, {"D", 4}}
  );
}

TEST(Serialization, JSONStdUnorderedStringMap) {
  check_field_serialization<std::unordered_map<std::string, int>, formats::json_fmt>(
    "{\"value\":{\"A\":1,\"B\":2,\"C\":3,\"D\":4}}", {{"A", 1}, {"B", 2}, {"C", 3}, {"D", 4}}
  );
}

TEST(Serialization, JSONStdSet) {
  check_field_serialization<std::set<int>, formats::json_fmt>(
    "{\"value\":[1,2,3,4]}", {1, 2, 3, 4}
  );
}

TEST(Serialization, JSONStdUnorderedSet) {
  check_field_serialization<std::unordered_set<int>, formats::json_fmt>(
           "{\"value\":[1,2,3,4]}", {1, 2, 3, 4}
         ) and check_field_serialization<std::unordered_set<int>, formats::json_fmt>(
           "{\"value\":[4,3,2,1]}", {1, 2, 3, 4}
         );
}

TEST(Serialization, JSONStdPair_Int_Int) {
  check_field_serialization<std::pair<int, int>, formats::json_fmt>(
    "{\"value\":[2,4]}", {2, 4}
  );
}

TEST(Serialization, JSONStdPair_String_Int) {
  check_field_serialization<std::pair<std::string, int>, formats::json_fmt>(
    "{\"value\":{\"hello, world!\":4}}", {"hello, world!", 4}
  );
}

//! Indirections

//! Shallow
TEST(Serialization, JSONPtr) {
  struct test_struct {
    int* ptr;
  };
  int i = 123;
  test_struct ts {&i};
  check_serializes_to<formats::json_fmt>(ts, "{\"ptr\":\"reference\"}");
}

TEST(Serialization, JSONConstPtr) {
  struct test_struct {
    const int* ptr;
  };
  int i = 123;
  test_struct ts {&i};
  check_serializes_to<formats::json_fmt>(ts, "{\"ptr\":\"reference\"}");
}

TEST(Serialization, JSONRef) {
  struct test_struct {
    int &ref;
  };
  int i = 123;
  test_struct ts {i};
  check_serializes_to<formats::json_fmt>(ts, "{\"ref\":\"reference\"}");
}

TEST(Serialization, JSONConstRef) {
  struct test_struct {
    const int &ref;
  };
  int i = 123;
  test_struct ts {i};
  check_serializes_to<formats::json_fmt>(ts, "{\"ref\":\"reference\"}");
}

TEST(Serialization, JSONStdSharedPtr) {
  struct test_struct {
    std::shared_ptr<int> ptr;
  };
  test_struct ts {std::make_shared<int>(123)};
  check_serializes_to<formats::json_fmt>(ts, "{\"ptr\":\"reference\"}");
}

TEST(Serialization, JSONStdUniquePtr) {
  struct test_struct {
    std::unique_ptr<int> value;
  };
  test_struct ts {std::make_unique<int>(123)};
  check_serializes_to<formats::json_fmt>(ts, "{\"value\":123}");
}

//! Deep
TEST(Serialization, JSONDeepPtr) {
  struct test_struct {
    [[meta(serialize::policy::deep)]]
    int* ptr;
  };
  int i = 123;
  test_struct ts {&i};
  check_serializes_to<formats::json_fmt>(ts, "{\"ptr\":123}");
}

TEST(Serialization, JSONDeepConstPtr) {
  struct test_struct {
    [[meta(serialize::policy::deep)]]
    const int* ptr;
  };
  int i = 123;
  test_struct ts {&i};
  check_serializes_to<formats::json_fmt>(ts, "{\"ptr\":123}");
}

TEST(Serialization, JSONDeepRef) {
  struct test_struct {
    [[meta(serialize::policy::deep)]]
    int &ref;
  };
  int i = 123;
  test_struct ts {i};
  check_serializes_to<formats::json_fmt>(ts, "{\"ref\":123}");
}

TEST(Serialization, JSONDeepConstRef) {
  struct test_struct {
    [[meta(serialize::policy::deep)]]
    const int &ref;
  };
  int i = 123;
  test_struct ts {i};
  check_serializes_to<formats::json_fmt>(ts, "{\"ref\":123}");
}

TEST(Serialization, JSONDeepStdSharedPtr) {
  struct test_struct {
    [[meta(serialize::policy::deep)]]
    std::shared_ptr<int> ptr;
  };
  test_struct ts {std::make_shared<int>(123)};
  check_serializes_to<formats::json_fmt>(ts, "{\"ptr\":123}");
}

TEST(Serialization, JSONChangeName) {
  struct test_struct {
    [[meta(serialize::name {"serialized name"})]]
    int in_memory_value = 123;
  } ts { };
  check_serializes_to<formats::json_fmt>(ts, "{\"serialized name\":123}");
}
