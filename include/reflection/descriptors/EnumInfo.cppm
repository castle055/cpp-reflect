/*! \file  EnumInfo.cppm
 *! \brief
 *!
 */

export module refl.descriptors:EnumInfo;

import std;


namespace refl {
  export struct TypeInfo;

  export struct EnumValue {
    std::uintmax_t magnitude;
    bool           is_negative;
  };

  export struct EnumeratorInfo {
    std::string_view name;
    EnumValue        value;
  };

  export struct EnumInfo {
    bool                        is_scoped;
    const TypeInfo             &underlying_type;
    std::vector<EnumeratorInfo> enumerators;
  };

  export template<typename E>
    requires std::is_enum_v<E>
  std::string_view enum_to_string(E it) {
    static constexpr auto enums = std::define_static_array(std::meta::enumerators_of(^^E));
    template for (constexpr auto &e: enums) {
      if ([:e:] == it) {
        return std::meta::identifier_of(e);
      }
    }
    return "<unknown enumerator>";
  }
} // namespace refl
