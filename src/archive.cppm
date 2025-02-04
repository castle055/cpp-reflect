/*! \file  any.cppm
 *! \brief 
 *!
 */

export module reflect:archive;

import std;

export import :any;

namespace refl {
  export class archive {
  public:
    archive() = default;

  public:
    any &operator[](const std::string &path) {
      if (not data_.contains(path)) {
        data_.emplace(path, any { });
      }
      return data_[path];
    }

  private:
    std::map<std::string, any> data_ { };
  };
}
