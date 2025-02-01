// Copyright (c) 2024, Víctor Castillo Agüero.
// SPDX-License-Identifier: GPL-3.0-or-later

/*! \file  pretty_print.cppm
 *! \brief
 *!
 */

export module reflect:pretty_print;

import std;

export import packtl;

import :types;
import :accessors;
import :type_name;

export namespace refl {
  using namespace packtl;

  template <refl::Reflected T, std::size_t I>
  void print_obj_field_impl(const T& obj, std::size_t indent) {
    static constexpr std::size_t field_count =
      get_size<typename T::__type_info__::field_types>::value;

    using f = refl::field<T, I>;

    for (std::size_t i = 0; i < indent; ++i) {
      std::cout << "  ";
    }

    switch (f::access) {
      case refl::access_spec::NONE:
        std::cout << "- ";
        break;
      case refl::access_spec::PRIVATE:
        std::cout << "X ";
        break;
      case refl::access_spec::PROTECTED:
        std::cout << "\\ ";
        break;
      case refl::access_spec::PUBLIC:
        std::cout << "  ";
        break;
    }

    std::cout << f::name << ": ";
    std::cout << type_name<typename f::type> << ";" << std::endl;
  }

  template <refl::Reflected T, std::size_t... I>
  void print_obj_impl(const T& obj, std::size_t indent, std::index_sequence<I...>) {
    for (std::size_t i = 0; i < indent; ++i) {
      std::cout << "  ";
    }

    std::cout << type_name<T> << " {" << std::endl;
    ((print_obj_field_impl<T, I>(obj, indent + 1)), ...);
    std::cout << "}" << std::endl;
  }

  struct pretty_print_opts {
    bool multiline = true;
  };

  template <refl::Reflected T>
  void pretty_print(const T& obj) {
    print_obj_impl<T>(obj, 0, std::make_index_sequence<field_count<T>>());
  }
}