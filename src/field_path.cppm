// Copyright (c) 2024-2025, Víctor Castillo Agüero.
// SPDX-License-Identifier: GPL-3.0-or-later

/*! \file  refl.cppm
 *! \brief
 *!
 */

export module reflect:field_path;

export import std;

export import :type_info;


namespace refl {
  export template<typename T>
  constexpr std::size_t offset_of(auto T::* member) {
    return reinterpret_cast<std::size_t>(
      &(reinterpret_cast<T const volatile*>(0)->*member)
    );
  }

  template<typename...>
  struct is_valid_path_t: std::false_type {
  };

  template<typename Prev, typename T, typename FT>
    requires (not std::same_as<Prev, T>)
  struct is_valid_path_t<Prev, FT T::*>: std::false_type {
    static_assert(std::same_as<Prev, T>, "Invalid path");
  };

  template<typename Prev, typename T, typename FT>
    requires std::same_as<Prev, T>
  struct is_valid_path_t<Prev, FT T::*>: std::true_type {
  };

  template<typename Prev, typename T, typename FT, typename... Rest>
    requires (not std::same_as<Prev, T>)
  struct is_valid_path_t<Prev, FT T::*, Rest...>: std::false_type {
  };

  template<typename Prev, typename T, typename FT, typename... Rest>
    requires std::same_as<Prev, T>
  struct is_valid_path_t<Prev, FT T::*, Rest...>: is_valid_path_t<FT, Rest...> {
  };

  export template<typename Root, typename... Fields>
  constexpr bool is_valid_path = is_valid_path_t<Root, Fields...>::value;

  export class field_path {
    const type_info* root_type_;
    std::vector<const field_info*> fields_;

  public:
    friend std::hash<field_path>;
    friend std::formatter<field_path>;

    field_path() = delete;

    template<typename... T>
    field_path(auto T::*... fields) requires (is_valid_path<typename packtl::get_first<T
                                                              ...>::type, decltype(fields)...>) : root_type_(
      &type_info::from<typename packtl::get_first<T...>::type>()) {
      const type_info* current = root_type_;
      ([&]<typename S>(auto S::* f) {
        auto f_info = current->field_by_offset(offset_of(f));
        if (f_info.has_value()) {
          fields_.push_back(f_info.value());
          current = &f_info.value()->type();
        }
      }(fields), ...);
    }

    explicit field_path(const type_info &root_type)
      : root_type_(&root_type) {
    }

    field_path(const type_info &root_type, const field_info* field)
      : root_type_(&root_type),
        fields_ {field} {
    }

    field_path(const type_info &root_type, std::initializer_list<const field_info*> fields)
      : root_type_(&root_type),
        fields_(fields) {
    }

    bool operator==(const field_path &other) const {
      return fields_ == other.fields_;
    }

    const type_info &type() const {
      return fields_.back()->type();
    }

    void* get_ptr(void* obj) const {
      void* ptr = obj;

      for (const field_info* field: fields_) {
        ptr = field->get_ptr(ptr);
      }

      return ptr;
    }

    const void* get_ptr(const void* obj) const {
      const void* ptr = obj;

      for (const field_info* field: fields_) {
        ptr = field->get_ptr(ptr);
      }

      return ptr;
    }

    template<typename T>
    T &get_ref(void* obj) const {
      return *static_cast<T*>(get_ptr(obj));
    }

    template<typename T>
    const T &get_ref(const void* obj) const {
      return *static_cast<const T*>(get_ptr(obj));
    }

    field_path append(const field_info* field) const {
      field_path fp {*root_type_};
      fp.fields_ = fields_;
      fp.fields_.emplace_back(field);
      return fp;
    }

    field_path append(const field_path &other) const {
      field_path fp {*root_type_};
      fp.fields_ = fields_;
      for (const auto &fi: other.fields_) {
        fp.fields_.emplace_back(fi);
      }
      return fp;
    }

    field_path parent() const {
      field_path fp {*root_type_};
      fp.fields_.resize(fields_.size() - 1);
      for (int i = 0; i < fields_.size() - 1; ++i) {
        fp.fields_[i] = fields_[i];
      }
      return fp;
    }

    field_path relative_to(const field_path &other) const {
      if (other.root_type() != root_type()) {
        throw std::logic_error(std::format("Paths of unrelated root types ('{}','{}')", root_type().name(),
                                           other.root_type().name()));
      }

      if (depth() <= other.depth()) {
        throw std::logic_error(std::format("Path with depth {} cannot be relative to path of depth {}", depth(),
                                           other.depth()));
      }

      bool valid = true;
      for (std::size_t i = 0; i < other.depth(); ++i) {
        if (fields_[i] != other.fields_[i]) {
          valid = false;
          break;
        }
      }

      if (not valid) {
        throw std::logic_error(std::format("Paths from different branches ({} and {})", this->to_string(),
                                           other.to_string()));
      }

      field_path fp {other.type()};
      fp.fields_.resize(depth() - other.depth());
      for (std::size_t i = other.depth() + 1, j = 0; i < depth(); ++i, ++j) {
        fp.fields_[j] = fields_[i];
        if (fields_[i] != other.fields_[i]) {
          valid = false;
          break;
        }
      }
      return fp;
    }

    std::size_t depth() const {
      return fields_.size();
    }

    const type_info &root_type() const {
      return *root_type_;
    }

    bool operator<(const field_path &other) const {
      if (other.root_type() != root_type()) {
        throw std::logic_error(std::format("Comparing paths of unrelated root types ('{}','{}')", root_type().name(),
                                           other.root_type().name()));
      }
      if (depth() == other.depth()) {
        return *this != other;
      }
      return depth() < other.depth();
    }

    auto front() const {
      return fields_.front();
    }

    auto back() const {
      return fields_.back();
    }

    auto begin() const {
      return fields_.begin();
    }

    auto end() const {
      return fields_.end();
    }

    std::string to_string() const {
      std::string ss;
      ss = std::format("({}){}::", depth(), root_type().name());
      for (int i = 0; i < fields_.size() - 1; ++i) {
        ss = std::format("{}{}.", ss, fields_[i]->name);
      }
      ss = std::format("{}{}", ss, fields_[fields_.size() - 1]->name);
      return ss;
    }

    static std::optional<field_path> from_string(const type_info &t_info, const std::string &str) {
      std::size_t dot_pos_prev       = 0;
      std::size_t dot_pos            = str.find('.');
      const type_info* current_tinfo = &t_info;
      std::string current_name       = str.substr(dot_pos_prev, dot_pos);
      field_path path {t_info};

      do {
        auto fi_opt = current_tinfo->field_by_name(current_name);
        if (not fi_opt.has_value()) {
          return std::nullopt;
        }

        path = path.append(fi_opt.value());

        dot_pos_prev  = dot_pos;
        dot_pos       = str.find('.', dot_pos_prev + 1);
        current_name  = str.substr(dot_pos_prev + 1, dot_pos);
        current_tinfo = &path.type();
      } while (dot_pos_prev != std::string::npos);

      return path;
    }
  };
}

export template<>
struct std::hash<refl::field_path> {
  std::size_t operator()(const refl::field_path &path) const noexcept {
    return std::hash<std::string> { }(path.to_string());
  }
};

template<>
struct std::formatter<refl::field_path> {
  constexpr auto parse(std::format_parse_context &ctx) {
    return ctx.begin();
  }

  template<typename FormatContext>
  auto format(const refl::field_path &p, FormatContext &ctx) {
    return std::format_to(ctx.out(), "{}", p.to_string());
  }
};
