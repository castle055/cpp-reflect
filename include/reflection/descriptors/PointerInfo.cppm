/*! \file  PointerInfo.cppm
 *! \brief 
 *!
 */

export module refl.descriptors:PointerInfo;

import std;


namespace refl {
  export struct TypeInfo;

  export struct PointerInfo {
    const TypeInfo &pointee;
  };

  export struct ReferenceInfo {
    const TypeInfo &pointee;
  };
}
