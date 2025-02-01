// Copyright (c) 2024-2025, Víctor Castillo Agüero.
// SPDX-License-Identifier: GPL-3.0-or-later

/*! \file  pretty_print.cppm
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
  template <template <typename> typename Format = formats::default_fmt>
  struct serializer {
    template <typename O>
    using args_t = typename Format<O>::args_t;

    static std::string to_string(const auto& obj, const args_t<std::stringstream>& args = {}) {
      std::stringstream str{};
      auto              format = Format<std::stringstream>{str, args};
      format.serialize(obj);
      return str.str();
    }

    template <typename O>
    static void to_stream(O& out, const auto& obj, const args_t<O>& args = {}) {
      auto format = Format<O>{out, args};
      format.serialize(obj);
    }
  };

  template <template <typename> typename Format = formats::default_fmt>
  std::string to_string(const auto& obj, const typename Format<std::stringstream>::args_t& args = {}) {
    return serializer<Format>::to_string(obj, args);
  }
} // namespace refl
