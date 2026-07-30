// Copyright (c) 2024-2025, Víctor Castillo Agüero.
// SPDX-License-Identifier: GPL-3.0-or-later

/*! \file  pretty_print.cppm
 *! \brief
 *!
 */

export module reflect.marshal.formats.default_fmt;

import std;

import packtl;
import reflect;

import reflect.marshal.formats.base;

export namespace formats {
  template <typename O>
  struct default_fmt: refl::visitor<default_fmt<O>> {
    struct args_t {
      bool         pretty = true;
      unsigned int indent = 2;
    };

    explicit default_fmt(O& out_, args_t args_): refl::visitor<default_fmt<O>>(), out(out_), args(args_) {}

    template <typename T>
    void handle_value(const T& it) {
      if constexpr (std::is_same_v<T, std::atomic_flag>) {
        out << (it.test() ? "SET" : "CLEAR");
      } else if constexpr (packtl::is_type<std::unique_ptr, T>::value) {
        const auto* value = it.get();
        out << " {";
        out << std::format("0x{:X}", (std::size_t)value);
        out << "} ";
      } else if constexpr (packtl::is_type<std::shared_ptr, T>::value) {
        const auto* value = it.get();
        out << "{";
        out << std::format("0x{:X}", (std::size_t)value);
        out << "} ";
      } else if constexpr (packtl::is_type<std::weak_ptr, T>::value) {
        const auto* value = it.get();
        out << " {";
        out << std::format("0x{:X}", (std::size_t)value);
        out << "}: ";
      } else if constexpr (refl::Reflected<T>) {
        if (visited_.contains((std::size_t)&it)) {
          out << "<circular reference>";
          return;
        }
        visited_.emplace((std::size_t)&it);
      } else if constexpr (std::is_convertible_v<T, std::string>) {
        out << std::format("{:?}", it);
      } else if constexpr (std::same_as<T, char>) {
        out << std::format("0x{:X} '{}'", (int)it, it);
      } else if constexpr (std::formattable<T, char>) {
        out << std::format("{}", it);
      }

      this->visit_value(it);
      out << ";";
    }

    template <typename T>
    void handle_iterable(const T& iterable) {
      this->visit_iterable(iterable);
    }

    template <typename T, typename Field>
    void handle_field(const T& obj) {
      switch (Field::access) {
        case refl::access_spec::NONE:
          out << " -";
          break;
        case refl::access_spec::PRIVATE:
          // out << "🔒";
          out << "🔒";
          break;
        // 🔓
        case refl::access_spec::PROTECTED:
          out << " \\";
          break;
        case refl::access_spec::PUBLIC:
          out << "  ";
          break;
        default:
          break;
      }

      out << Field::name;
      out << ": " << refl::type_name<typename Field::type> << " = ";

      this->template visit_obj_field<T, Field>(obj);
    }

    template <typename T>
    void handle_obj(const T& obj) {
      out << refl::type_name<T> << " {" << "\n";
      this->visit_obj(obj);
      // for (std::size_t i = 0; i < indent; ++i) {
      //   for (std::size_t j = 0; j < args.indent; ++j) {
      //     out << " ";
      //   }
      // }
      out << "}";
    }

    template <refl::Reflected R>
    void serialize(const R& obj) {
      this->visit(obj);
      out << "\n";
    }

  private:
    std::unordered_set<std::size_t> visited_{};
    O&                              out;
    args_t args;
  };
} // namespace formats
