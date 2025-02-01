// Copyright (c) 2024-2025, Víctor Castillo Agüero.
// SPDX-License-Identifier: GPL-3.0-or-later

/*! \file  accessors.cppm
 *! \brief
 *!
 */

export module reflect:accessors;

import std;

import packtl;

import :types;
import :type_name;

template <typename Type, std::size_t Offset>
struct representation {
  unsigned char offset_[Offset];
  Type          value;
};

template <typename Type>
struct representation<Type, 0> {
  Type value;
};


export namespace refl {

  enum class access_spec : unsigned char {
    NONE      = 0U,
    PRIVATE   = 1U,
    PROTECTED = 2U,
    PUBLIC    = 3U,
  };

  template <refl::Reflected T, std::size_t I>
  struct field {
    static constexpr std::size_t index = I;
    static constexpr const char* name  = static_type_info<T>::field_names[I];
    using type = typename packtl::get<I, typename static_type_info<T>::field_types>::type;
    static constexpr std::size_t size =
      packtl::get<I, typename static_type_info<T>::field_sizes>::value;
    static constexpr std::size_t offset =
      packtl::get<I, typename static_type_info<T>::field_offsets>::value;
    static constexpr access_spec access =
      access_spec{packtl::get<I, typename static_type_info<T>::field_access_specifiers>::value};

    static constexpr bool is_reference = std::is_reference_v<type>;
    static constexpr bool is_pointer   = std::is_pointer_v<type>;

    static constexpr std::size_t metadata_offset =
      packtl::get<I, typename static_type_info<T>::field_metadata_offsets>::value;
    static constexpr std::size_t metadata_count =
      packtl::get<I, typename static_type_info<T>::field_metadata_counts>::value;

    template <std::size_t J>
      requires(J < metadata_count)
    using metadata_type = typename std::
      tuple_element<metadata_offset + J, decltype(static_type_info<T>::field_metadata)>::type;

    template <std::size_t J>
      requires(J < metadata_count)
    static constexpr decltype(std::get<metadata_offset + J>(static_type_info<T>::field_metadata)
    ) metadata_item = std::get<metadata_offset + J>(static_type_info<T>::field_metadata);

    template <typename MetadataType>
    static constexpr bool has_metadata = []<std::size_t... J>(std::index_sequence<J...>) -> bool {
      if constexpr ((std::same_as<const MetadataType, metadata_type<J>> or ...)) {
        return true;
      } else {
        return false;
      }
    }(std::make_index_sequence<metadata_count>{});

    template <typename MetadataType, std::size_t StartFrom>
      requires(not has_metadata<MetadataType>)
    static int find_metadata() {
      return 0;
    }

    template <typename MetadataType, std::size_t StartFrom>
      requires(has_metadata<MetadataType> and StartFrom < metadata_count)
    static consteval MetadataType find_metadata() {
      if constexpr (std::same_as<const MetadataType, metadata_type<StartFrom>>) {
        return metadata_item<StartFrom>;
      } else {
        return find_metadata<MetadataType, StartFrom + 1>();
      }
    }

    template <typename MetadataType>
    static constexpr MetadataType get_metadata = find_metadata<MetadataType, 0>();

    static const std::remove_reference_t<type>& from_instance(const T& instance) {
      const auto* rep = reinterpret_cast<const representation<type, offset>*>(&instance);
      return rep->value;
    }

    static std::remove_reference_t<type>& from_instance(T& instance) {
      auto* rep = reinterpret_cast<representation<type, offset>*>(&instance);
      return rep->value;
    }
  };

  template <refl::Reflected T>
  constexpr std::size_t field_count =
    packtl::get_size<typename static_type_info<T>::field_types>::value;

  template <refl::Reflected T, std::size_t I>
  constexpr decltype(std::get<I>(static_type_info<T>::field_metadata)) field_meta =
    std::get<I>(static_type_info<T>::field_metadata);

  template <refl::Reflected T, std::size_t I>
  struct method {
    static constexpr std::size_t index = I;
    static constexpr const char* name  = static_type_info<T>::method_names[I];
    using type = typename packtl::get<I, typename static_type_info<T>::method_types>::type;
    static constexpr access_spec access =
      access_spec{packtl::get<I, typename static_type_info<T>::method_access_specifiers>::value};
  };

  template <refl::Reflected T>
  constexpr std::size_t method_count =
    packtl::get_size<typename static_type_info<T>::method_types>::value;

  template <Reflected R, template <Reflected, typename> typename Fun, typename... Args>
  auto for_each_field(Args&&... args) {
    static constexpr auto count = field_count<R>;

    auto impl = [&]<std::size_t... I>(std::index_sequence<I...>) {
      return ((Fun<R, field<R, I>>(args...)) && ...);
    };

    return impl(std::make_index_sequence<count>{});
  }
} // namespace refl
