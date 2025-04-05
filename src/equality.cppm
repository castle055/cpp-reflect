// Copyright (c) 2024-2025, Víctor Castillo Agüero.
// SPDX-License-Identifier: GPL-3.0-or-later

/*! \file  pretty_print.cppm
 *! \brief
 *!
 */

export module reflect:equality;

import std;

import packtl;

import :types;
import :accessors;

export namespace refl::eq_policy {
  enum policy_e {
    shallow,
    deep,
    skip
  };
}

export namespace refl {
  template <Reflected R>
  bool deep_eq(const R& lhs, const R& rhs);
}

namespace refl::deep_eq_impl {
  template <typename T>
  bool ref_eq(const T& lhs, const T& rhs);

  template <typename I>
  bool std_iterable_eq(const I& lhs, const I& rhs) {
    using T = typename I::value_type;

    if (lhs.size() != rhs.size()) {
      return false;
    }
    for (auto it1 = lhs.begin(), it2 = rhs.begin(); it1 != lhs.end() && it2 != rhs.end();
         ++it1, ++it2) {
      if (not ref_eq<T>(*it1, *it2)) {
        return false;
      }
    }
    return true;
  }

  template <typename P>
  bool std_pair_eq(const P& lhs, const P& rhs) {
    using T1 = typename P::first_type;
    using T2 = typename P::second_type;

    if (not ref_eq<T1>(lhs.first, rhs.first)) {
      return false;
    }
    if (not ref_eq<T2>(lhs.second, rhs.second)) {
      return false;
    }

    return true;
  }

  template <typename I>
  bool std_map_eq(const I& lhs, const I& rhs) {
    using T = typename I::value_type;

    if (lhs.size() != rhs.size()) {
      return false;
    }
    for (auto it1 = lhs.begin(), it2 = rhs.begin(); it1 != lhs.end() && it2 != rhs.end();
         ++it1, ++it2) {
      if (not std_pair_eq<T>(*it1, *it2)) {
        return false;
      }
    }
    return true;
  }

  template <typename F>
  bool std_function_eq(const F& lhs, const F& rhs) {
    // I know this isn't correct, but properly comparing an std::function is impossible in C++
    return lhs.target_type() == rhs.target_type();
  }

  template <typename T>
  bool ref_eq(const T& lhs, const T& rhs) {
    if constexpr (packtl::is_type<std::vector, T>::value) {
      return std_iterable_eq(lhs, rhs);
    } else if constexpr (packtl::is_type<std::list, T>::value) {
      return std_iterable_eq(lhs, rhs);
    } else if constexpr (packtl::is_type<std::deque, T>::value) {
      return std_iterable_eq(lhs, rhs);
    } else if constexpr (packtl::is_type<std::queue, T>::value) {
      return std_iterable_eq(lhs.__get_container(), rhs.__get_container());
    } else if constexpr (packtl::is_type<std::stack, T>::value) {
      return std_iterable_eq(lhs, rhs);
    } else if constexpr (packtl::is_type<std::map, T>::value) {
      return std_map_eq(lhs, rhs);
    } else if constexpr (packtl::is_type<std::unordered_map, T>::value) {
      return std_map_eq(lhs, rhs);
    } else if constexpr (packtl::is_type<std::set, T>::value) {
      return std_iterable_eq(lhs, rhs);
    } else if constexpr (packtl::is_type<std::unordered_set, T>::value) {
      return std_iterable_eq(lhs, rhs);
    } else if constexpr (packtl::is_type<std::pair, T>::value) {
      return std_pair_eq(lhs, rhs);
    } else if constexpr (packtl::is_type<std::function, T>::value) {
      return std_function_eq(lhs, rhs);
    } else if constexpr (std::equality_comparable<T>) {
      return lhs == rhs;
    } else if constexpr (Reflected<T>) {
      return deep_eq(lhs, rhs);
    } else {
      return false;
    }
  }

  template <Reflected R, typename field_data>
  bool field_eq(const R& lhs, const R& rhs) {
    const auto& field1 = field_data::from_instance(lhs);
    const auto& field2 = field_data::from_instance(rhs);

    eq_policy::policy_e policy{eq_policy::deep};
    if constexpr (field_data::template has_metadata<eq_policy::policy_e>) {
      policy = field_data::template get_metadata<eq_policy::policy_e>;
    }

    if (policy == eq_policy::skip) {
      return true;
    }

    if constexpr (field_data::is_reference) {
      if (policy == eq_policy::deep) {
        if (&field1 == &field2) {
          return true;
        }

        using field_type = std::remove_reference_t<typename field_data::type>;
        return ref_eq<field_type>(field1, field2);
      } else if (policy == eq_policy::shallow) {
        return &field1 == &field2;
      }
    } else if constexpr (field_data::is_pointer) {
      if (policy == eq_policy::deep) {
        if (field1 == field2) {
          return true;
        }

        using field_type = std::remove_pointer_t<typename field_data::type>;
        if constexpr (std::is_void_v<field_type>) {
          return false;
        } else {
          return ref_eq<field_type>(*field1, *field2);
        }
      } else if (policy == eq_policy::shallow) {
        return field1 == field2;
      }
    } else {
      using field_type = typename field_data::type;
      return ref_eq<field_type>(field1, field2);
    }

    return false;
  }
} // namespace refl::deep_eq_impl

namespace refl {
  template <Reflected R>
  bool deep_eq(const R& lhs, const R& rhs) {
    static constexpr auto count = field_count<R>;

    auto impl = [&]<std::size_t... I>(std::index_sequence<I...>) {
      return ((deep_eq_impl::field_eq<R, field<R, I>>(lhs, rhs)) && ...);
    };

    return impl(std::make_index_sequence<count>{});
  }
}