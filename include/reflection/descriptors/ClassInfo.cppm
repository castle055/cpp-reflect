/*! \file  ClassInfo.cppm
 *! \brief
 *!
 */

export module refl.descriptors:ClassInfo;

import std;

import :FieldInfo;


namespace refl {
  export struct BaseInfo {
    TypeGetter type;
    AccessSpec access;
    bool       is_virtual;
  };

  export struct ClassInfo {
    std::vector<BaseInfo>  base_classes{};
    std::vector<FieldInfo> fields{};
  };
} // namespace refl
