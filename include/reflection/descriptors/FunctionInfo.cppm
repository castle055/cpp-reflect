/*! \file  FunctionInfo.cppm
 *! \brief
 *!
 */

export module refl.descriptors:FunctionInfo;

import std;


namespace refl {
  export struct TypeInfo;

  export struct FunctionInfo {
    const TypeInfo               &return_type;
    std::vector<const TypeInfo *> parameter_types;

    bool is_noexcept;
    bool is_variadic;
  };
} // namespace refl
