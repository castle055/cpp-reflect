
// Copyright (c) 2024-2025, Víctor Castillo Agüero.
// SPDX-License-Identifier: GPL-3.0-or-later

// #include <cassert>
#include "common.h"

import reflect;

import packtl;
import reflect.serialize;

void setup() {
}

struct serialize_me {
  int           a    = 5;
  std::string   str  = "hello there!";
  serialize_me* next = nullptr;
};

TEST("Serialization") {
  serialize_me sm1{};
  serialize_me sm{};
  sm.next = &sm1;

  refl::serializer<formats::json_fmt>::to_stream(std::cout, sm);

  std::cout << refl::to_string<formats::json_fmt>(sm);

  return 0;
}

template <template <typename> typename Format, typename T>
int check_serializes_to(const T& t, const std::string& serialized) {
  std::string str = refl::serializer<Format>::to_string(t);

  if (refl::field_count<T> >= 0) {
    [&]<std::size_t... I>(std::index_sequence<I...>) {
      ((std::cout << "FIELD TYPE: "
                  << std::format("{:?}", refl::type_name<typename refl::field<T, I>::type>)
                  << std::endl),
       ...);
    }(std::make_index_sequence<refl::field_count<T>>{});
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

TEST("JSON Empty") {
  struct test_struct {
  } test_obj{};
  return check_serializes_to<formats::json_fmt>(test_obj, "{}");
}


template <typename Field>
struct test_one_field_struct {
  Field value;
};

template <typename Field, template <typename> typename Format>
int check_field_serialization(const std::string& serialized, Field default_value = {}) {
  test_one_field_struct<Field> test_obj{};
  test_obj.value = default_value;
  return check_serializes_to<Format>(test_obj, serialized);
}

TEST("JSON Int Zero") {
  return check_field_serialization<int, formats::json_fmt>("{\"value\":0}");
}

TEST("JSON short Zero") {
  return check_field_serialization<short, formats::json_fmt>("{\"value\":0}");
}

TEST("JSON Long Zero") {
  return check_field_serialization<long, formats::json_fmt>("{\"value\":0}");
}

TEST("JSON Unsigned Int Zero") {
  return check_field_serialization<unsigned int, formats::json_fmt>("{\"value\":0}");
}

TEST("JSON Unsigned Short Zero") {
  return check_field_serialization<unsigned short, formats::json_fmt>("{\"value\":0}");
}

TEST("JSON Unsigned Long Zero") {
  return check_field_serialization<unsigned long, formats::json_fmt>("{\"value\":0}");
}

TEST("JSON Float Zero") {
  return check_field_serialization<float, formats::json_fmt>("{\"value\":0.0}");
}

TEST("JSON Double Zero") {
  return check_field_serialization<double, formats::json_fmt>("{\"value\":0.0}");
}

TEST("JSON Bool True") {
  return check_field_serialization<bool, formats::json_fmt>("{\"value\":\"true\"}", true);
}

TEST("JSON Bool False") {
  return check_field_serialization<bool, formats::json_fmt>("{\"value\":\"false\"}", false);
}

TEST("JSON Char") {
  return check_field_serialization<char, formats::json_fmt>("{\"value\":\"c\"}", 'c');
}

TEST("JSON Char Pointer String") {
  char str[] = "hello, world!";
  return check_field_serialization<char*, formats::json_fmt>("{\"value\":\"hello, world!\"}", str);
}

TEST("JSON Const Char Pointer String") {
  return check_field_serialization<const char*, formats::json_fmt>(
    "{\"value\":\"hello, world!\"}", "hello, world!"
  );
}

TEST("JSON Std String") {
  return check_field_serialization<std::string, formats::json_fmt>(
    "{\"value\":\"hello, world!\"}", "hello, world!"
  );
}

TEST("JSON Std String View") {
  return check_field_serialization<std::string_view, formats::json_fmt>(
    "{\"value\":\"hello, world!\"}", "hello, world!"
  );
}

TEST("JSON Std Vector") {
  return check_field_serialization<std::vector<int>, formats::json_fmt>(
    "{\"value\":[1,2,3,4]}", {1, 2, 3, 4}
  );
}

TEST("JSON Std Array") {
  return check_field_serialization<std::array<int, 4>, formats::json_fmt>(
    "{\"value\":[1,2,3,4]}", {1, 2, 3, 4}
  );
}

TEST("JSON Std List") {
  return check_field_serialization<std::list<int>, formats::json_fmt>(
    "{\"value\":[1,2,3,4]}", {1, 2, 3, 4}
  );
}
TEST("JSON Std Deque") {
  return check_field_serialization<std::deque<int>, formats::json_fmt>(
    "{\"value\":[1,2,3,4]}", {1, 2, 3, 4}
  );
}
TEST("JSON Std Int Map") {
  return check_field_serialization<std::map<int, int>, formats::json_fmt>(
    "{\"value\":[[1,1],[2,2],[3,3],[4,4]]}", {{1, 1}, {2, 2}, {3, 3}, {4, 4}}
  );
}
TEST("JSON Std Unordered Int Map") {
  return check_field_serialization<std::unordered_map<int, int>, formats::json_fmt>(
    "{\"value\":[[1,1],[2,2],[3,3],[4,4]]}", {{1, 1}, {2, 2}, {3, 3}, {4, 4}}
  );
}
TEST("JSON Std String Map") {
  return check_field_serialization<std::map<std::string, int>, formats::json_fmt>(
    "{\"value\":{\"A\":1,\"B\":2,\"C\":3,\"D\":4}}", {{"A", 1}, {"B", 2}, {"C", 3}, {"D", 4}}
  );
}
TEST("JSON Std Unordered String Map") {
  return check_field_serialization<std::unordered_map<std::string, int>, formats::json_fmt>(
    "{\"value\":{\"A\":1,\"B\":2,\"C\":3,\"D\":4}}", {{"A", 1}, {"B", 2}, {"C", 3}, {"D", 4}}
  );
}
TEST("JSON Std Set") {
  return check_field_serialization<std::set<int>, formats::json_fmt>(
    "{\"value\":[1,2,3,4]}", {1, 2, 3, 4}
  );
}
TEST("JSON Std Unordered Set") {
  return check_field_serialization<std::unordered_set<int>, formats::json_fmt>(
    "{\"value\":[1,2,3,4]}", {1, 2, 3, 4}
  );
}
TEST("JSON Std Pair <Int, Int>") {
  return check_field_serialization<std::pair<int, int>, formats::json_fmt>(
    "{\"value\":[2,4]}", {2, 4}
  );
}
TEST("JSON Std Pair <String, Int>") {
  return check_field_serialization<std::pair<std::string, int>, formats::json_fmt>(
    "{\"value\":{\"hello, world!\":4}}", {"hello, world!", 4}
  );
}

//! Indirections

//! Shallow
TEST("JSON Ptr") {
  struct test_struct {
    int* ptr;
  };
  int i = 123;
  test_struct ts{&i};
  return check_serializes_to<formats::json_fmt>(ts, "{\"ptr\":\"reference\"}");
}
TEST("JSON Const Ptr") {
  struct test_struct {
    const int* ptr;
  };
  int i = 123;
  test_struct ts{&i};
  return check_serializes_to<formats::json_fmt>(ts, "{\"ptr\":\"reference\"}");
}
TEST("JSON Ref") {
  struct test_struct {
    int& ref;
  };
  int i = 123;
  test_struct ts{i};
  return check_serializes_to<formats::json_fmt>(ts, "{\"ref\":\"reference\"}");
}
TEST("JSON Const Ref") {
  struct test_struct {
    const int& ref;
  };
  int i = 123;
  test_struct ts{i};
  return check_serializes_to<formats::json_fmt>(ts, "{\"ref\":\"reference\"}");
}
TEST("JSON Std Shared Ptr") {
  struct test_struct {
    std::shared_ptr<int> ptr;
  };
  test_struct ts{std::make_shared<int>(123)};
  return check_serializes_to<formats::json_fmt>(ts, "{\"ptr\":\"reference\"}");
}
TEST("JSON Std Unique Ptr") {
  struct test_struct {
    std::unique_ptr<int> value;
  };
  test_struct ts{std::make_unique<int>(123)};
  return check_serializes_to<formats::json_fmt>(ts, "{\"value\":123}");
}

//! Deep
TEST("JSON Deep Ptr") {
  struct test_struct {
    [[meta(serialize::policy::deep)]]
    int* ptr;
  };
  int i = 123;
  test_struct ts{&i};
  return check_serializes_to<formats::json_fmt>(ts, "{\"ptr\":123}");
}
TEST("JSON Deep Const Ptr") {
  struct test_struct {
    [[meta(serialize::policy::deep)]]
    const int* ptr;
  };
  int i = 123;
  test_struct ts{&i};
  return check_serializes_to<formats::json_fmt>(ts, "{\"ptr\":123}");
}
TEST("JSON Deep Ref") {
  struct test_struct {
    [[meta(serialize::policy::deep)]]
    int& ref;
  };
  int i = 123;
  test_struct ts{i};
  return check_serializes_to<formats::json_fmt>(ts, "{\"ref\":123}");
}
TEST("JSON Deep Const Ref") {
  struct test_struct {
    [[meta(serialize::policy::deep)]]
    const int& ref;
  };
  int i = 123;
  test_struct ts{i};
  return check_serializes_to<formats::json_fmt>(ts, "{\"ref\":123}");
}
TEST("JSON Deep Std Shared Ptr") {
  struct test_struct {
    [[meta(serialize::policy::deep)]]
    std::shared_ptr<int> ptr;
  };
  test_struct ts{std::make_shared<int>(123)};
  return check_serializes_to<formats::json_fmt>(ts, "{\"ptr\":123}");
}

TEST("JSON Change Name") {
  struct test_struct {
    [[meta(serialize::name {"serialized name"})]]
    int in_memory_value = 123;
  } ts{};
  return check_serializes_to<formats::json_fmt>(ts, "{\"serialized name\":123}");
}
