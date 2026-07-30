/*! \file  FieldInfo.cppm
 *! \brief
 *!
 */

export module refl.descriptors:FieldInfo;

import std;


namespace refl {
  export struct TypeInfo;

  export using TypeGetter = const TypeInfo & (*) ();

  export enum class AccessSpec {
    PRIVATE,
    PROTECTED,
    PUBLIC,
  };

  export struct FieldInfo {
    std::string_view         name;
    std::meta::member_offset offset;
    TypeGetter               type;
  };
} // namespace refl
