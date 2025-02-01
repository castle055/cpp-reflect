// Copyright (c) 2024-2025, Víctor Castillo Agüero.
// SPDX-License-Identifier: GPL-3.0-or-later

/*! \file  pretty_print.cppm
 *! \brief
 *!
 */

export module reflect:visitor;

import std;

import packtl;

import :types;
import :accessors;
import :type_name;

export namespace refl {
  template <typename T>
  struct is_std_array: std::false_type {};

  template <typename T, std::size_t N>
  struct is_std_array<std::array<T, N>>: std::true_type {};

  template <typename Derived>
  struct visitor {
    visitor() = default;
    virtual ~visitor() {}

    template <Reflected R>
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
      visit_reference(obj);
    }

    template <typename T>
    void handle_pointer(const T* obj) {
      visit_pointer(obj);
    }

    template <typename T, typename Field>
    void handle_field(const T& obj) {
      visit_obj_field<T, Field>(obj);
    }

    template <typename T>
    void handle_iterable_element(const T& iterable) {
      visit_iterable_element(iterable);
    }

    template <typename T>
    void handle_iterable(const T& iterable) {
      visit_iterable(iterable);
    }

    template <typename T>
    void handle_tuple_element(const T& iterable) {
      visit_tuple_element(iterable);
    }

    template <typename T>
    void handle_tuple(const T& iterable) {
      visit_tuple(iterable);
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

    template <typename T>
    void visit_reference(const T& obj) {
      self().handle_value(obj);
    }

    template <typename T>
    void visit_pointer(const T* obj) {
      if constexpr (not std::is_void_v<T>) {
        if (obj != nullptr) {
          self().handle_value(*obj);
        }
      }
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
//      using item_type = typename T::value_type;
      for (auto item = iterable.begin(); item != iterable.end(); ++item) {
        self().handle_iterable_element(*item);
      }
    }

    template <typename T>
    void visit_iterable_element(const T& item) {
      using item_type = T;
      if constexpr (std::is_reference_v<item_type>) {
        const auto& it = item;
        self().handle_reference(it);
      } else if constexpr (std::is_pointer_v<item_type>) {
        const auto* it = item;
        self().handle_pointer(it);
      } else {
        const auto& it = item;
        self().handle_value(it);
      }
    }

    template <typename T>
    void visit_tuple(const T& it) {
      [&]<std::size_t... I>(std::index_sequence<I...>) {
        (self().handle_tuple_element(std::get<I>(it)), ...);
      }(std::make_index_sequence<std::tuple_size_v<T>>{});
    }

    template <typename T>
    void visit_tuple_element(const T& item) {
      using item_type = T;
      if constexpr (std::is_reference_v<item_type>) {
        const auto& it = item;
        self().handle_reference(it);
      } else if constexpr (std::is_pointer_v<item_type>) {
        const auto* it = item;
        self().handle_pointer(it);
      } else {
        const auto& it = item;
        self().handle_value(it);
      }
    }

  private:
    auto& self() {
      return *static_cast<Derived*>(this);
    }

    template <refl::Reflected R, std::size_t I>
    void visit_obj_method(const R& obj) {
//      using m = refl::method<R, I>;
    }

  protected:
    template <typename T>
    void visit_any(const T& it) {
      if constexpr (std::same_as<T, char*>) {
        return;
      } else if constexpr (std::same_as<T, const char*>) {
        return;
      } else if constexpr (std::is_reference_v<T>) {
        self().handle_reference(it);
      } else if constexpr (std::is_pointer_v<T>) {
        self().handle_pointer(it);
      }  else if constexpr (packtl::is_type<std::vector, T>::value) {
        self().handle_iterable(it);
      }  else if constexpr (is_std_array<T>::value) {
        self().handle_iterable(it);
      } else if constexpr (packtl::is_type<std::list, T>::value) {
        self().handle_iterable(it);
      } else if constexpr (packtl::is_type<std::deque, T>::value) {
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
        self().handle_tuple(it);
      // } else if constexpr (packtl::is_type<std::unique_ptr, T>::value) {
      //   const auto* value = it.get();
      //   self().handle_value(*value);
      // } else if constexpr (packtl::is_type<std::shared_ptr, T>::value) {
      //   const auto* value = it.get();
      //   self().handle_value(*value);
      } else if constexpr (packtl::is_type<std::weak_ptr, T>::value) {
        if (not it.expired()) {
          const auto* value = it.lock().get();
          self().handle_value(*value);
        }
      } else if constexpr (refl::Reflected<T>) {
        self().handle_obj(it);
      }
    }
  };
} // namespace refl
