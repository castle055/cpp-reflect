// Copyright (c) 2024-2025, Víctor Castillo Agüero.
// SPDX-License-Identifier: GPL-3.0-or-later

/*! \file  serializer.cppm
 *! \brief
 *!
 */

export module reflect.serialize;

import std;

import packtl;

export import reflect;
export import reflect.marshal.formats.base;
export import reflect.marshal.formats.default_fmt;
export import reflect.marshal.formats.json;

export namespace refl {
  template<template <typename> typename Format = formats::json_fmt>
  struct serializer {
    template<typename O>
    using args_t = typename Format<O>::args_t;

    static std::string to_string(const auto &obj, const args_t<std::stringstream> &args = { }) {
      std::stringstream str { };
      auto format = Format<std::stringstream> {str, args};
      format.serialize(obj);
      return str.str();
    }

    template<typename O>
    static void to_stream(O &out, const auto &obj, const args_t<O> &args = { }) {
      auto format = Format<O> {out, args};
      format.serialize(obj);
    }
  };

  std::string to_json(const auto &obj, const formats::json_fmt<std::stringstream>::args_t &args = { }) {
    return serializer<formats::json_fmt>::to_string(obj, args);
  }
} // namespace refl
