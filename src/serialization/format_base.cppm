// Copyright (c) 2024-2025, Víctor Castillo Agüero.
// SPDX-License-Identifier: GPL-3.0-or-later

/*! \file  pretty_print.cppm
 *! \brief
 *!
 */

export module reflect.marshal.formats.base;

import std;

import packtl;
import reflect;

export namespace serialize::policy {
  enum policy_e {
    shallow,
    deep,
    skip
  };
  template <typename F>
  struct handle {
    F func;
  };
}
export namespace serialize {
  struct name {
    std::string value;
  };
}


export namespace formats {
  template <typename Format, typename Args>
  struct base {
    using args_t = Args;
    args_t args;

    explicit base(args_t args_)
        : args(args_) {}

    virtual ~base() {}

    template <typename O, refl::Reflected R, std::size_t I>
    void print_obj_field_impl(O& out, const R& obj, std::size_t indent) {
      using f = refl::field<R, I>;

      for (std::size_t i = 0; i < std::max((indent * args.indent), 0UL); ++i) {
        out << " ";
      }

      switch (f::access) {
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
      }

      out << f::name;
      // using raw_field_type = std::remove_pointer_t<std::remove_reference_t<typename f::type>>;
      // if constexpr (not refl::Reflected<typename f::type>) {
      // out << " [" << type_name<typename f::type> << "]";
      out << ": " << refl::type_name<typename f::type> << " = ";
      // }

      // if constexpr (f::access == field_access::PUBLIC) {
      if constexpr (std::is_reference_v<typename f::type>) {
        const auto& it = f::from_instance(obj);
        out << " {";
        out << std::format("0x{:X}", (std::size_t)&it);
        out << "} ";

        // if constexpr (refl::Reflected<std::remove_reference_t<typename f::type>>) {

        print_any(out, it, indent);
        // }
      } else if constexpr (std::is_pointer_v<typename f::type>) {
        const auto* it = f::from_instance(obj);
        out << " {";
        out << std::format("0x{:X}", (std::size_t)it);
        out << "} ";

        // if constexpr (refl::Reflected<std::remove_reference_t<typename f::type>>) {
        if constexpr (not std::is_void_v<std::remove_pointer_t<typename f::type>>) {
          if (it != nullptr) {
            print_any(out, *it, indent);
          }
        }
      } else {
        if constexpr (not refl::Reflected<typename f::type>) {
          out << " { ";
        }
        const auto& it = f::from_instance(obj);
        // const typename f::type& it =
        //   *(const typename f::type*)(((unsigned char*)&obj) + f::offset);
        print_any(out, it, indent);
        if constexpr (not refl::Reflected<typename f::type>) {
          out << " }";
        }
      }
      // }

      out << std::endl;
    }

    template <typename O, refl::Reflected R, std::size_t... I>
    void print_obj_impl(O& out, const R& obj, std::size_t indent, std::index_sequence<I...>) {
      out << refl::type_name<R> << " {" << "\n";
      ((print_obj_field_impl<O, R, I>(out, obj, indent + 1)), ...);
      for (std::size_t i = 0; i < indent; ++i) {
        for (std::size_t j = 0; j < args.indent; ++j) {
          out << " ";
        }
      }
      out << "}";
    }

    template <typename O, refl::Reflected R, std::size_t I>
    void print_obj_method(O& out, const R& obj) {
      using m = refl::method<R, I>;

      switch (m::access) {
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
      }

      out << m::name;
      out << ": " << refl::type_name<typename m::type>;
    }

    template <typename O, refl::Reflected R>
    void print_obj(O& out, const R& obj, std::size_t indent = 0) {
      print_obj_impl(out, obj, indent, std::make_index_sequence<refl::field_count<R>>());

      [&]<std::size_t... I>(std::index_sequence<I...>) {
        (print_obj_method<O, R, I>(out, obj), ...);
      }(std::make_index_sequence<refl::method_count<R>>());
    }

    template <typename O, typename T>
    void print_std_iterable(O& out, const T& it, std::size_t indent) {
      using item_type = typename T::value_type;
      out << "[" << std::endl;
      for (auto item = it.begin(); item != it.end(); ++item) {
        for (std::size_t i = 0; i < std::max((indent * args.indent), 0UL); ++i) {
          out << " ";
        }

        if constexpr (std::is_reference_v<item_type>) {
          const auto& value = *item;
          out << " {";
          out << std::format("0x{:X}", (std::size_t)&value);
          out << "} ";

          // if constexpr (refl::Reflected<std::remove_reference_t<typename f::type>>) {

          print_any(out, value, indent + 1);
          // }
        } else if constexpr (std::is_pointer_v<item_type>) {
          const auto* value = *item;
          out << " {";
          out << std::format("0x{:X}", (std::size_t)value);
          out << "} ";

          // if constexpr (refl::Reflected<std::remove_reference_t<typename f::type>>) {
          if constexpr (not std::is_void_v<std::remove_pointer_t<item_type>>) {
            if (value != nullptr) {
              print_any(out, *value, indent + 1);
            }
          }
        } else {
          if constexpr (refl::Reflected<item_type>) {
            out << "  ";
          } else {
            out << " {";
          }
          const auto& value = *item;
          // const typename f::type& it =
          //   *(const typename f::type*)(((unsigned char*)&obj) + f::offset);
          print_any(out, value, indent + 1);
          if constexpr (not refl::Reflected<item_type>) {
            out << " }";
          }
        }
        out << "," << std::endl;
      }
      for (std::size_t i = 0; i < std::max((indent * args.indent), 0UL); ++i) {
        out << " ";
      }
      out << "]";
    }

    template <typename O, typename T>
    void print_std_pair(O& out, const T& it, std::size_t indent) {
      using item_type_1 = typename T::first_type;
      using item_type_2 = typename T::second_type;
      out << "[";

      if constexpr (std::is_reference_v<item_type_1>) {
        const auto& value = it.first;
        out << " {";
        out << std::format("0x{:X}", (std::size_t)&value);
        out << "} ";

        // if constexpr (refl::Reflected<std::remove_reference_t<typename f::type>>) {

        print_any(out, value, indent + 1);
        // }
      } else if constexpr (std::is_pointer_v<item_type_1>) {
        const auto* value = it.first;
        out << " {";
        out << std::format("0x{:X}", (std::size_t)value);
        out << "} ";

        // if constexpr (refl::Reflected<std::remove_reference_t<typename f::type>>) {
        if constexpr (not std::is_void_v<std::remove_pointer_t<item_type_1>>) {
          if (value != nullptr) {
            out << ": ";

            print_any(out, *value, indent + 1);
          }
        }
      } else {
        if constexpr (not refl::Reflected<item_type_1>) {
          out << "{";
        }
        const auto& value = it.first;
        // const typename f::type& it =
        //   *(const typename f::type*)(((unsigned char*)&obj) + f::offset);
        print_any(out, value, indent + 1);
        if constexpr (not refl::Reflected<item_type_1>) {
          out << "}";
        }
      }
      out << ", ";
      if constexpr (std::is_reference_v<item_type_2>) {
        const auto& value = it.second;
        out << " {";
        out << std::format("0x{:X}", (std::size_t)&value);
        out << "} ";

        // if constexpr (refl::Reflected<std::remove_reference_t<typename f::type>>) {

        print_any(out, value, indent);
        // }
      } else if constexpr (std::is_pointer_v<item_type_2>) {
        const auto* value = it.second;
        out << " {";
        out << std::format("0x{:X}", (std::size_t)value);
        out << "} ";

        // if constexpr (refl::Reflected<std::remove_reference_t<typename f::type>>) {
        if constexpr (not std::is_void_v<std::remove_pointer_t<item_type_2>>) {
          if (value != nullptr) {
            print_any(out, *value, indent);
          }
        }
      } else {
        if constexpr (not refl::Reflected<item_type_2>) {
          out << "{";
        }
        const auto& value = it.second;
        // const typename f::type& it =
        //   *(const typename f::type*)(((unsigned char*)&obj) + f::offset);
        print_any(out, value, indent);
        if constexpr (not refl::Reflected<item_type_2>) {
          out << "}";
        }
      }
      out << "]";
    }

    template <typename O, typename T>
    void print_any(O& out, const T& it, std::size_t indent) {
      if constexpr (packtl::is_type<std::vector, T>::value) {
        print_std_iterable(out, it, indent);
      } else if constexpr (packtl::is_type<std::list, T>::value) {
        print_std_iterable(out, it, indent);
      } else if constexpr (packtl::is_type<std::deque, T>::value) {
        print_std_iterable(out, it, indent);
      } else if constexpr (packtl::is_type<std::queue, T>::value) {
        print_std_iterable(out, it, indent);
      } else if constexpr (packtl::is_type<std::stack, T>::value) {
        print_std_iterable(out, it, indent);
      } else if constexpr (packtl::is_type<std::map, T>::value) {
        print_std_iterable(out, it, indent);
      } else if constexpr (packtl::is_type<std::unordered_map, T>::value) {
        print_std_iterable(out, it, indent);
      } else if constexpr (packtl::is_type<std::set, T>::value) {
        print_std_iterable(out, it, indent);
      } else if constexpr (packtl::is_type<std::unordered_set, T>::value) {
        print_std_iterable(out, it, indent);
      } else if constexpr (packtl::is_type<std::pair, T>::value) {
        print_std_pair(out, it, indent);
      } else if constexpr (std::is_same_v<T, std::atomic_flag>) {
        out << (it.test() ? "SET" : "CLEAR");
      } else if constexpr (packtl::is_type<std::unique_ptr, T>::value) {
        const auto* value = it.get();
        out << " {";
        out << std::format("0x{:X}", (std::size_t)value);
        out << "} ";
        print_any(out, *value, indent);
      } else if constexpr (packtl::is_type<std::shared_ptr, T>::value) {
        const auto* value = it.get();
        out << "{";
        out << std::format("0x{:X}", (std::size_t)value);
        out << "} ";
        print_any(out, *value, indent);
      } else if constexpr (packtl::is_type<std::weak_ptr, T>::value) {
        const auto* value = it.get();
        out << " {";
        out << std::format("0x{:X}", (std::size_t)value);
        out << "}: ";
        print_any(out, *value, indent);
      } else if constexpr (refl::Reflected<T>) {
        if (visited_.contains((std::size_t)&it)) {
          out << "<circular reference>";
          return;
        }
        visited_.emplace((std::size_t)&it);
        print_obj(out, it, indent);
      } else if constexpr (std::is_convertible_v<T, std::string>) {
        out << std::format("{:?}", it);
      } else if constexpr (std::same_as<T, char>) {
        out << std::format("0x{:X} '{}'", (int)it, it);
      } else if constexpr (std::formattable<T, char>) {
        out << std::format("{}", it);
      }
    }

    template <refl::Reflected R>
    void visit(const R& obj) {
      self().handle_obj(obj);
    }

  private:
    template <typename T>
    void handle_obj(const T& obj) {
      visit_obj(obj);
    }

    template <typename T>
    void handle_reference(const T& obj) {
      self().handle_value(obj);
    }

    template <typename T>
    void handle_pointer(const T* obj) {
      if constexpr (not std::is_void_v<T>) {
        if (obj != nullptr) {
          self().handle_value(*obj);
        }
      }
    }

    template <typename T, typename Field>
    void handle_field(const T& obj) {
      visit_obj_field<T, Field>(obj);
    }

    template <typename T>
    void handle_iterable(const T& iterable) {
      visit_iterable(iterable);
    }

    template <typename T>
    void handle_value(const T& value) {
      visit_value(value);
    }

  protected:
    template <typename T>
    void visit_value(const T& obj) {
      visit_any(obj);
    }

    template <refl::Reflected R>
    void visit_obj(const R& obj) {
      [&]<std::size_t... I>(std::index_sequence<I...>) {
        (self().template handle_field<R, refl::field<R, I>>(obj), ...);
      }(std::make_index_sequence<refl::field_count<R>>());

      [&]<std::size_t... I>(std::index_sequence<I...>) {
        (visit_obj_method<R, I>(obj), ...);
      }(std::make_index_sequence<refl::method_count<R>>());
    }

    template <refl::Reflected R, typename Field>
    void visit_obj_field(const R& obj) {
      if constexpr (std::is_reference_v<typename Field::type>) {
        const auto& it = Field::from_instance(obj);
        self().handle_reference(it);
      } else if constexpr (std::is_pointer_v<typename Field::type>) {
        const auto* it = Field::from_instance(obj);
        self().handle_pointer(it);
      } else {
        const auto& it = Field::from_instance(obj);
        self().handle_value(it);
      }
    }

    template <typename T>
    void visit_iterable(const T& iterable) {
      using item_type = typename T::value_type;
      for (auto item = iterable.begin(); item != iterable.end(); ++item) {
        if constexpr (std::is_reference_v<item_type>) {
          const auto& it = *item;
          self().handle_reference(it);
        } else if constexpr (std::is_pointer_v<item_type>) {
          const auto* it = *item;
          self().handle_pointer(it);
        } else {
          const auto& it = *item;
          self().handle_value(it);
        }
      }
    }

  private:
    auto& self() {
      return *static_cast<Format*>(this);
    }

    template <typename T>
    void visit_std_pair(const T& it) {}

    template <refl::Reflected R, std::size_t I>
    void visit_obj_method(const R& obj) {
//      using m = refl::method<R, I>;
    }

  protected:
    template <typename T>
    void visit_any(const T& it) {
      if constexpr (packtl::is_type<std::vector, T>::value) {
        self().handle_iterable(it);
      } else if constexpr (packtl::is_type<std::list, T>::value) {
        self().handle_iterable(it);
      } else if constexpr (packtl::is_type<std::deque, T>::value) {
        self().handle_iterable(it);
      } else if constexpr (packtl::is_type<std::queue, T>::value) {
        self().handle_iterable(it);
      } else if constexpr (packtl::is_type<std::stack, T>::value) {
        self().handle_iterable(it);
      } else if constexpr (packtl::is_type<std::map, T>::value) {
        self().handle_iterable(it);
      } else if constexpr (packtl::is_type<std::unordered_map, T>::value) {
        self().handle_iterable(it);
      } else if constexpr (packtl::is_type<std::set, T>::value) {
        self().handle_iterable(it);
      } else if constexpr (packtl::is_type<std::unordered_set, T>::value) {
        self().handle_iterable(it);
      } else if constexpr (packtl::is_type<std::pair, T>::value) {
        self().visit_std_pair(it);
        // } else if constexpr (std::is_same_v<T, std::atomic_flag>) {
        //   out << (it.test() ? "SET" : "CLEAR");
        // } else if constexpr (packtl::is_type<std::unique_ptr, T>::value) {
        //   const auto* value = it.get();
        //   out << " {";
        //   out << std::format("0x{:X}", (std::size_t)value);
        //   out << "} ";
        //   print_any(out, *value, indent);
        // } else if constexpr (packtl::is_type<std::shared_ptr, T>::value) {
        //   const auto* value = it.get();
        //   out << "{";
        //   out << std::format("0x{:X}", (std::size_t)value);
        //   out << "} ";
        //   print_any(out, *value, indent);
        // } else if constexpr (packtl::is_type<std::weak_ptr, T>::value) {
        //   const auto* value = it.get();
        //   out << " {";
        //   out << std::format("0x{:X}", (std::size_t)value);
        //   out << "}: ";
        //   print_any(out, *value, indent);
        } else if constexpr (refl::Reflected<T>) {
          self().handle_obj(it);
        // } else if constexpr (std::is_convertible_v<T, std::string>) {
        //   out << std::format("{:?}", it);
        // } else if constexpr (std::same_as<T, char>) {
        //   out << std::format("0x{:X} '{}'", (int)it, it);
        // } else if constexpr (std::formattable<T, char>) {
        //   out << std::format("{}", it);
      }
    }

  private:
    std::unordered_set<std::size_t> visited_{};
  };
} // namespace formats
