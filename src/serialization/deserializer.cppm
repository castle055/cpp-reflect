// Copyright (c) 2024-2025, Víctor Castillo Agüero.
// SPDX-License-Identifier: GPL-3.0-or-later

/*! \file  deserializer.cppm
 *! \brief
 *!
 */

export module reflect.deserialize;

import std;

import packtl;

export import reflect;
export import reflect.marshal.formats.base;
export import reflect.marshal.formats.default_fmt;
export import reflect.marshal.formats.json;

export namespace refl {
  struct deserializer_options {
    bool ignore_missing  = true;
    bool ignore_null     = true;
    bool truncate_arrays = true;
  };
}

namespace refl::detail {
  template<typename T>
  void map_to_value(T &obj, const any &value, const deserializer_options &opt);

  template<typename T>
  void map_to_object(T &obj, const archive &value, const deserializer_options &opt) {
    static constexpr auto count = field_count<T>;
    [&]<std::size_t... I>(std::index_sequence<I...>) {
      ([&]<std::size_t J>() {
        using field = field<T, J>;
        auto &ref   = field::from_instance(obj);
        if (not value.contains(field::name)) {
          if (not opt.ignore_missing) {
            throw std::runtime_error(std::format("Missing required field '{}::{}'", type_name<T>, field::name));
          }
        } else {
          map_to_value(ref, value.at(field::name), opt);
        }
      }.template operator()<I>(), ...);
    }(std::make_index_sequence<count> { });
  }

  template<typename T, std::size_t N>
  void map_to_iterable(std::array<T, N> &obj, const std::vector<any> &value, const deserializer_options &opt) {
    std::size_t i = 0;
    for (T &it: obj) {
      if (i >= value.size()) {
        if (not opt.truncate_arrays) {
          throw std::runtime_error(std::format("Truncating fixed size array '{}'", type_name<std::array<T, N>>));
        }
        return;
      }
      map_to_value(it, value.at(i), opt);
      ++i;
    }
  }

  template<typename T>
  void map_to_iterable(std::vector<T> &obj, const std::vector<any> &value, const deserializer_options &opt) {
    obj.resize(value.size());
    std::size_t i = 0;
    for (T &it: obj) {
      map_to_value(it, value.at(i), opt);
      ++i;
    }
  }

  template<typename T>
  void map_to_iterable(std::list<T> &obj, const std::vector<any> &value, const deserializer_options &opt) {
    obj.clear();
    for (const any &v: value) {
      obj.emplace_back();
      T &it = obj.back();
      map_to_value(it, v, opt);
    }
  }

  template<typename T>
  void map_to_iterable(std::deque<T> &obj, const std::vector<any> &value, const deserializer_options &opt) {
    obj.clear();
    for (const any &v: value) {
      obj.emplace_back();
      T &it = obj.back();
      map_to_value(it, v, opt);
    }
  }

  template<typename T, typename AnyT>
  bool map_to_primitive(T &it, const any &value, const deserializer_options &opt) {
    if (not value.is<AnyT>()) {
      return false;
    }
    if constexpr (std::convertible_to<AnyT, T>) {
      it = value.as<AnyT>();
    } else {
      throw std::invalid_argument(std::format("value type mismatch, expected '{}', found '{}'", type_name<T>,
                                              type_name<AnyT>));
    }
    return true;
  }

  template<typename T>
  void map_to_value(T &obj, const any &value, const deserializer_options &opt) {
    if (value.is<std::string>() and value.as<std::string>() == "null") {
      if (not opt.ignore_null) {
        throw std::invalid_argument("null value");
      }
      return;
    }

    if constexpr (Reflected<T>) {
      static constexpr auto count = field_count<T>;
      if (not value.is<archive>()) {
        throw std::invalid_argument("value is not an archive");
      }
      map_to_object(obj, value.as<archive>(), opt);
    } else if constexpr ((not std::same_as<T, std::string>) and requires(T t)
    {
      t.end(); t.begin(); typename T::value_type;
    }) {
      if (not value.is<std::vector<any>>()) {
        throw std::invalid_argument("value is not a list");
      }
      map_to_iterable(obj, value.as<std::vector<any>>(), opt);
    } else {
      const bool ok = map_to_primitive<T, std::int64_t>(obj, value, opt)
                      or map_to_primitive<T, std::uint64_t>(obj, value, opt)
                      or map_to_primitive<T, std::double_t>(obj, value, opt)
                      or map_to_primitive<T, std::string>(obj, value, opt);
      if (not ok) {
        throw std::invalid_argument(std::format("value is not a known primitive: {}", "hello"));
        //, value.type().name()));
      }
    }
  }
}

export namespace refl {
  template<template <typename> typename Format = formats::json_fmt>
  struct deserializer {
    template<typename O>
    using args_t = typename Format<O>::args_t;

    static any from_string(const std::string_view &str) {
      return Format<std::stringstream>::deserialize(str);
    }

    static any from_json(const JSON &obj) {
      return Format<std::stringstream>::deserialize(obj);
    }
  };

  // Works with string, string_views, etc...
  template<typename T = any, typename StringView = std::string_view>
    requires (not std::same_as<StringView, JSON>)
  T from_json(const StringView &obj, const deserializer_options &opt = { }) {
    static_assert(Reflected<T>, "Type is not reflected");
    if constexpr (std::same_as<T, any>) {
      return deserializer<formats::json_fmt>::from_string(obj);
    } else {
      T t { };
      any value = deserializer<formats::json_fmt>::from_string(obj);
      detail::map_to_value(t, value, opt);
      return t;
    }
  }

  // Works only with refl::JSON
  template<typename T = any>
  T from_json(const JSON &obj, const deserializer_options &opt = { }) {
    static_assert(Reflected<T>, "Type is not reflected");
    if constexpr (std::same_as<T, any>) {
      return deserializer<formats::json_fmt>::from_json(obj);
    } else {
      T t { };
      any value = deserializer<formats::json_fmt>::from_json(obj);
      detail::map_to_value(t, value, opt);
      return t;
    }
  }

} // namespace refl
