// Copyright (c) 2024-2025, Víctor Castillo Agüero.
// SPDX-License-Identifier: GPL-3.0-or-later

/*! \file  _types.cppm
 *! \brief
 *!
 */

export module reflect:types;

import std;

export namespace refl {
  template <typename T>
  using static_type_info = typename T::__type_info__;

  template <typename T>
  concept is_type_info = requires {
    typename T::field_types;
    typename T::field_sizes;
    typename T::field_offsets;
  };

  template <typename T>
  concept Reflected = requires {
    // typename T::__type_info__;
    is_type_info<typename T::__type_info__>;
  };

  using type_id_t = std::size_t;
} // namespace refl

export {
  template <typename... T>
  struct refl_pack;
  template <unsigned long... T>
  struct refl_int_pack;

  template <typename... T>
  using refl_tuple = std::tuple<T...>;
}
