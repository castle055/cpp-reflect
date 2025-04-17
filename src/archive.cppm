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

    any &at(const std::string &path) {
      return data_.at(path);
    }

    const any &at(const std::string &path) const {
      return data_.at(path);
    }

    bool contains(const std::string &path) const {
      return data_.contains(path);
    }
  private:
    std::map<std::string, any> data_ { };
  };
}
