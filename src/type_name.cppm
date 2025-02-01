// Copyright (c) 2024-2025, Víctor Castillo Agüero.
// SPDX-License-Identifier: GPL-3.0-or-later

/*! \file  type_name.cppm
 *! \brief
 *!
 */

module;
#include <cstddef>

#if defined(__clang__) || defined(__GNUC__)
#define REFL_PRETTY_FUNCTION __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#define REFL_PRETTY_FUNCTION __FUNCSIG__
#else
#error "unsupported compiler (pretty function)"
#endif

export module reflect:type_name;

import std;

import :types;

export namespace refl {
  namespace detail { // Yoinked from JDH's implementation
    // type_name adapted from:
    // https://bitwizeshift.github.io/posts/2021/03/09/getting-an-unmangled-type-name-at-compile-time/
    template <std::size_t... Is>
    constexpr auto substring_as_array(std::string_view s, std::index_sequence<Is...>) {
      return std::array{s[Is]..., '\0'};
    }

    template <typename T>
    constexpr auto type_name_array() {
#if defined(__clang__)
      constexpr auto prefix = std::string_view{"[T = "};
      constexpr auto suffix = std::string_view{"]"};
#elif defined(__GNUC__)
      constexpr auto prefix = std::string_view{"with T = "};
      constexpr auto suffix = std::string_view{"]"};
#elif defined(_MSC_VER)
      constexpr auto prefix = std::string_view{"type_name_array<"};
      constexpr auto suffix = std::string_view{">(void)"};
#else
#error "unsupported compiler (type_name_array)"
#endif

      constexpr auto function = std::string_view{REFL_PRETTY_FUNCTION};
      constexpr auto start    = function.find(prefix) + prefix.size();
      constexpr auto end      = function.rfind(suffix);

      static_assert(start < end);

      constexpr auto name = function.substr(start, (end - start));
      constexpr auto size = name.size();
      // As of right now, this next line shows an error in the IDE because it thinks
      // that 'size' is maybe not known at compile time. Making this function a consteval
      // didn't help. Don't know how to silence this. It compiles fine.
      return substring_as_array(name, std::make_index_sequence<size>{});
    }
    template <template <typename...> typename Pack>
    constexpr auto type_name_array_pack() {
#if defined(__clang__)
      constexpr auto prefix = std::string_view{"[Pack = "};
      constexpr auto suffix = std::string_view{"]"};
#elif defined(__GNUC__)
      constexpr auto prefix = std::string_view{"with T = "};
      constexpr auto suffix = std::string_view{"]"};
#elif defined(_MSC_VER)
      constexpr auto prefix = std::string_view{"type_name_array<"};
      constexpr auto suffix = std::string_view{">(void)"};
#else
#error "unsupported compiler (type_name_array)"
#endif

      constexpr auto function = std::string_view{REFL_PRETTY_FUNCTION};
      constexpr auto start    = function.find(prefix) + prefix.size();
      constexpr auto end      = function.rfind(suffix);

      static_assert(start < end);

      constexpr auto name = function.substr(start, (end - start));
      constexpr auto size = name.size();
      // As of right now, this next line shows an error in the IDE because it thinks
      // that 'size' is maybe not known at compile time. Making this function a consteval
      // didn't help. Don't know how to silence this. It compiles fine.
      return substring_as_array(name, std::make_index_sequence<size>{});
    }
    template <template <typename T, std::size_t I> typename Pack>
    constexpr auto type_name_array_pack_1t1i() {
#if defined(__clang__)
      constexpr auto prefix = std::string_view{"[T = "};
      constexpr auto suffix = std::string_view{"]"};
#elif defined(__GNUC__)
      constexpr auto prefix = std::string_view{"with T = "};
      constexpr auto suffix = std::string_view{"]"};
#elif defined(_MSC_VER)
      constexpr auto prefix = std::string_view{"type_name_array<"};
      constexpr auto suffix = std::string_view{">(void)"};
#else
#error "unsupported compiler (type_name_array)"
#endif

      constexpr auto function = std::string_view{REFL_PRETTY_FUNCTION};
      constexpr auto start    = function.find(prefix) + prefix.size();
      constexpr auto end      = function.rfind(suffix);

      static_assert(start < end);

      constexpr auto name = function.substr(start, (end - start));
      constexpr auto size = name.size();
      // As of right now, this next line shows an error in the IDE because it thinks
      // that 'size' is maybe not known at compile time. Making this function a consteval
      // didn't help. Don't know how to silence this. It compiles fine.
      return substring_as_array(name, std::make_index_sequence<size>{});
    }


    template <typename T>
    struct type_name_holder {
      static inline constexpr auto value = type_name_array<T>();
    };
    template <template <typename...> typename Pack>
    struct type_name_holder_pack {
      static inline constexpr auto value = type_name_array_pack<Pack>();
    };
    template <template <typename T, std::size_t I> typename Pack>
    struct type_name_holder_pack_1t1i {
      static inline constexpr auto value = type_name_array_pack_1t1i<Pack>();
    };


    template <typename T>
    constexpr auto type_name_str() {
      constexpr auto& val = type_name_holder<T>::value;
      return std::string_view{val.data(), val.size() - 1};
    }
    template <template <typename...> typename Pack>
    constexpr auto type_name_str_pack() {
      constexpr auto& val = type_name_holder_pack<Pack>::value;
      return std::string_view{val.data(), val.size() - 1};
    }
    template <template <typename T, std::size_t I> typename Pack>
    constexpr auto type_name_str_pack_1t1i() {
      constexpr auto& val = type_name_holder_pack_1t1i<Pack>::value;
      return std::string_view{val.data(), val.size() - 1};
    }


    constexpr std::uint64_t fnv1a_partial(std::uint64_t partial, std::string_view s) {
      if (s.length() == 0) {
        return 0;
      }

      partial = (partial ^ s[0]) * 1099511628211u;
      return s.length() == 1 ? partial : fnv1a_partial(partial, s.substr(1));
    }

    // constexpr FNV1a hash
    constexpr std::uint64_t fnv1a_append(std::uint64_t partial, std::string_view s) {
      if (s.length() >= 256) {
        return fnv1a_append(fnv1a_partial(partial, s.substr(0, 256)), s.substr(256));
      } else {
        return fnv1a_partial(partial, s);
      }
    }

    constexpr std::uint64_t fnv1a(std::string_view s) {
      return fnv1a_append(14695981039346656037u, s);
    }

  } // namespace detail

  template <typename T>
  constexpr auto type_name = detail::type_name_str<T>();

  template <template <typename...> typename Pack>
  constexpr auto pack_name = detail::type_name_str_pack<Pack>();

  template <template <typename T, std::size_t I> typename Pack>
  constexpr auto pack_1t1i_name = detail::type_name_str_pack_1t1i<Pack>();

  template <typename T>
  constexpr type_id_t type_id = detail::fnv1a(type_name<T>);

  template <template <typename...> typename Pack>
  constexpr type_id_t pack_id = detail::fnv1a(pack_name<Pack>);

  template <template <typename T, std::size_t I> typename Pack>
  constexpr type_id_t pack_1t1i_id = detail::fnv1a(pack_1t1i_name<Pack>);

  namespace detail {
    template <typename T>
    struct pack_type_id {
      static constexpr type_id_t value = type_id<T>;
    };
    template <template <typename...> typename Pack, typename... Ts>
    struct pack_type_id<Pack<Ts...>> {
      static constexpr type_id_t value = pack_id<Pack>;
    };
    template <template <typename, std::size_t> typename Pack, typename T, std::size_t I>
    struct pack_type_id<Pack<T, I>> {
      static constexpr type_id_t value = pack_1t1i_id<Pack>;
    };
  }

  template <typename T>
  constexpr type_id_t pack_type_id = detail::pack_type_id<T>::value;
}