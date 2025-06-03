// Copyright (c) 2024-2025, Víctor Castillo Agüero.
// SPDX-License-Identifier: GPL-3.0-or-later

/*! \file  refl.cppm
 *! \brief
 *!
 */

export module reflect:type_info;

export import std;

export import :types;
export import :type_name;
export import :accessors;
export import :equality;


namespace refl {
  export class type_info;
  std::map<type_id_t, type_info> type_registry { };

  type_id_t get_id_from_info_getter(const type_info & (* tif)());

  export struct field_info {
    std::size_t index;
    std::string name;
    std::size_t size;
    std::size_t offset;
    access_spec access_type;
    type_id_t type_id;

    [[refl::ignore]]
    const type_info & (* type)();

    std::vector<std::pair<const type_info& (*)(), void*>> metadata;

    // Accessors
    void* get_ptr(void* obj) const {
      return static_cast<char*>(obj) + offset;
    }

    const void* get_ptr(const void* obj) const {
      return static_cast<const char*>(obj) + offset;
    }

    template<typename T>
    T &get_ref(void* obj) const {
      return *static_cast<T*>(get_ptr(obj));
    }

    template<typename T>
    const T &get_ref(const void* obj) const {
      return *static_cast<const T*>(get_ptr(obj));
    }

    template<typename MetadataType>
    bool has_metadata() const {
      static constexpr type_id_t t_id = refl::type_id<const MetadataType>;
      for (const auto &[tif, ptr]: metadata) {
        if (t_id == get_id_from_info_getter(tif)) {
          return true;
        }
      }
      return false;
    }

    template<typename MetadataType>
    const MetadataType &get_metadata() const {
      static constexpr type_id_t t_id = refl::type_id<const MetadataType>;
      for (const auto &[tif, ptr]: metadata) {
        if (t_id == get_id_from_info_getter(tif)) {
          return *static_cast<const MetadataType*>(ptr);
        }
      }
      throw std::runtime_error("Could not find metadata type");
    }
  };

  export struct method_info {
    std::size_t index;
    std::string name;
    access_spec access_type;
  };

  export template<typename T>
  struct get_pack_param_ids {
    static std::vector<type_id_t> vector() {
      return { };
    }
  };

  template<template <typename...> typename Pack, typename... Args>
  struct get_pack_param_ids<Pack<Args...>> {
    static std::vector<type_id_t> vector();
  };

  template<template <typename, std::size_t> typename Pack, typename T, std::size_t I>
  struct get_pack_param_ids<Pack<T, I>> {
    static std::vector<type_id_t> vector();
  };

  class type_info {
  private:
    template<typename Type>
    static const type_info &type_getter() {
      return from<Type>();
    }

    template<typename Field>
    static inline field_info make_field_data() {
      field_info field {
        .index = Field::index,
        .name = Field::name,
        .size = Field::size,
        .offset = Field::offset,
        .access_type = Field::access,
        .type_id = Field::type_id,
        .type = &type_getter<typename Field::type>,
      };
      field.metadata.resize(Field::metadata_count);

      [&]<std::size_t... I>(std::index_sequence<I...>) {
        ((field.metadata.at(I) = {
            &type_getter<typename Field::template metadata_type<I>>,
            static_cast<void*>(new std::remove_const_t<typename Field::template metadata_type<I>> {
              Field::template metadata_item<I>
            })
          }),
          ...
        );
      }(std::make_index_sequence<Field::metadata_count> { });

      return field;
    }

    template<typename Field>
    inline void push_field() {
      fields_.push_back(make_field_data<Field>());
      const auto &field = fields_.back();

      fields_by_name_.insert_or_assign(field.name, &field);
      fields_by_offset_.insert_or_assign(field.offset, &field);
    }

    template<typename Method>
    inline void push_method() {
      methods_.push_back({
        // .type        = []() -> type_info { return from<typename field<T, I>::type>(); },
        .index = Method::index,
        .name = Method::name,
        .access_type = Method::access,
      });
      const auto &method = methods_.back();

      methods_by_name_.insert_or_assign(method.name, &method);
    }

  public:
    template<typename T>
    static inline const type_info &from() {
      using type                     = std::remove_const_t<std::remove_reference_t<T>>;
      static constexpr type_id_t tid = type_id<T>;
      static constexpr type_id_t pid = pack_type_id<T>;

      if (type_registry.contains(tid)) {
        return type_registry.at(tid);
      }

      type_info &ti = type_registry[tid];
      ti.type_id_   = tid;
      ti.name_      = type_name<T>;

      if constexpr (std::is_lvalue_reference_v<T>) {
        ti.is_lval_ref_      = true;
        ti.indirect_type_id_ = from<std::remove_reference_t<T>>().id();
      } else if constexpr (std::is_rvalue_reference_v<T>) {
        ti.is_rval_ref_      = true;
        ti.indirect_type_id_ = from<std::remove_reference_t<T>>().id();
      } else if constexpr (std::is_pointer_v<T>) {
        ti.is_ptr_           = true;
        ti.indirect_type_id_ = from<std::remove_pointer_t<T>>().id();
      } else {
        ti.indirect_type_id_ = std::nullopt;
      }

      if constexpr (std::is_const_v<T>) {
        ti.is_const_ = true;
      }

      if constexpr (std::is_lvalue_reference_v<T> or std::is_rvalue_reference_v<T> or std::is_pointer_v<T>) {
        ti.copy_construct_function_ = [](const void* src) -> void* {
          const T &src_ref = *static_cast<const T*>(src);
          T* dest          = new T(src_ref);
          return dest;
        };

        if (ti.is_ptr()) {
          ti.copy_assign_function_ = [](void* dest, const void* src) {
            type &dest_ref      = *static_cast<type*>(dest);
            const type &src_ref = *static_cast<const type*>(src);
            dest_ref            = src_ref;
          };
        }

        ti.equality_function_ = [](const void* lhs, const void* rhs) {
          const T &LHS = *static_cast<const T*>(lhs);
          const T &RHS = *static_cast<const T*>(rhs);
          return LHS == RHS;
        };
      } else {
        if constexpr (Reflected<type>) {
          static constexpr std::size_t f_count = field_count<type>;
          static constexpr std::size_t m_count = method_count<type>;

          [&]<std::size_t... I>(std::index_sequence<I...>) {
            (ti.push_field<field<type, I>>(), ...);
          }(std::make_index_sequence<f_count> { });

          [&]<std::size_t... I>(std::index_sequence<I...>) {
            (ti.push_method<method<type, I>>(), ...);
          }(std::make_index_sequence<m_count> { });
        }

        if constexpr (pid != 0) {
          ti.pack_id_        = pid;
          ti.pack_param_ids_ = get_pack_param_ids<type>::vector();
        }

        if constexpr (std::is_copy_constructible_v<T>) {
          ti.copy_construct_function_ = [](const void* src) -> void* {
            const type &src_ref = *static_cast<const type*>(src);
            type* dest          = new type(src_ref);
            return dest;
          };
        }

        if constexpr (std::is_copy_assignable_v<type>) {
          if (not ti.is_const_) {
            ti.copy_assign_function_ = [](void* dest, const void* src) {
              type &dest_ref      = *static_cast<type*>(dest);
              const type &src_ref = *static_cast<const type*>(src);
              dest_ref            = src_ref;
            };
          }
        }

        if constexpr (Reflected<type>) {
          ti.equality_function_ = [](const void* lhs, const void* rhs) {
            const type &LHS = *static_cast<const type*>(lhs);
            const type &RHS = *static_cast<const type*>(rhs);
            return deep_eq(LHS, RHS);
          };
        } else if constexpr (std::equality_comparable<type> and not std::is_function_v<type>) {
          ti.equality_function_ = [](const void* lhs, const void* rhs) {
            const type &LHS = *static_cast<const type*>(lhs);
            const type &RHS = *static_cast<const type*>(rhs);
            return LHS == RHS;
          };
        }
      }

      return ti;
    }

    bool operator==(const type_info &other) const {
      return type_id_ == other.type_id_;
    }

    const std::string &name() const {
      return name_;
    }

    const auto &fields() const {
      return fields_;
    }

    std::optional<const field_info*> field_by_name(const std::string &name) const {
      if (fields_by_name_.contains(name)) {
        return fields_by_name_.at(name);
      }
      return std::nullopt;
    }

    std::optional<const field_info*> field_by_offset(const std::size_t &offset) const {
      if (fields_by_offset_.contains(offset)) {
        return fields_by_offset_.at(offset);
      }
      return std::nullopt;
    }

    std::size_t hash_code() const {
      return type_id_;
    }

    std::size_t id() const {
      return type_id_;
    }

    template<typename T>
    bool is_type() const {
      static type_id_t tid = type_id<T>;
      return type_id_ == tid;
    }

    template<template <typename...> typename Pack>
    bool is_pack() const {
      static type_id_t pid = pack_id<Pack>;
      return pack_id_ == pid;
    }

    template<template <typename T, std::size_t S> typename Pack>
    bool is_pack_1t1i() const {
      static type_id_t pid = pack_1t1i_id<Pack>;
      return pack_id_ == pid;
    }

    bool is_const() const {
      return is_const_;
    }

    bool is_indirect() const {
      // return is_lval_ref() || is_rval_ref() || is_ptr();
      return indirect_type_id_.has_value();
    }

    bool is_rval_ref() const {
      return is_rval_ref_;
    }

    bool is_lval_ref() const {
      return is_lval_ref_;
    }

    bool is_ptr() const {
      return is_ptr_;
    }

    const type_info &indirect_type() const {
      if (indirect_type_id_.has_value()) {
        return type_registry[indirect_type_id_.value()];
      } else {
        return type_registry[0];
      }
    }

    std::vector<const type_info*> pack_parameter_types() const {
      std::vector<const type_info*> tis { };
      for (const auto &tid: pack_param_ids_) {
        tis.emplace_back(&type_registry[tid]);
      }
      return tis;
    }

    void* make_copy_of(const void* ptr) const {
      if (nullptr != copy_construct_function_) {
        return copy_construct_function_(ptr);
      }
      return nullptr;
    }

    void assign_copy_of(const void* src, void* dest) const {
      if (nullptr != copy_assign_function_) {
        copy_assign_function_(dest, src);
      }
    }

    bool equality(const void* lhs, const void* rhs) const {
      if (nullptr != equality_function_) {
        return equality_function_(lhs, rhs);
      }
      return false;
    }

  private:
    std::string name_ { };
    std::list<field_info> fields_ { };
    std::unordered_map<std::string, const field_info*> fields_by_name_ { };
    std::unordered_map<std::size_t, const field_info*> fields_by_offset_ { };
    std::list<method_info> methods_ { };
    std::unordered_map<std::string, const method_info*> methods_by_name_ { };

    bool is_const_    = false;
    bool is_lval_ref_ = false;
    bool is_rval_ref_ = false;
    bool is_ptr_      = false;

    type_id_t type_id_ { };
    std::optional<type_id_t> indirect_type_id_ {std::nullopt};
    type_id_t pack_id_ { };
    std::vector<type_id_t> pack_param_ids_ { };

    [[refl::ignore]]
    void*(* copy_construct_function_)(const void*) {nullptr};

    [[refl::ignore]]
    void (* copy_assign_function_)(void*, const void*) {nullptr};

    [[refl::ignore]]
    bool (* equality_function_)(const void*, const void*) {nullptr};
  };


  template<template <typename...> typename Pack, typename... Args>
  std::vector<type_id_t> get_pack_param_ids<Pack<Args...>>::vector() {
    std::vector<type_id_t> ids { };
    (ids.push_back(type_info::from<Args>().id()), ...);
    return ids;
  }

  template<template <typename, std::size_t> typename Pack, typename T, std::size_t I>
  std::vector<type_id_t> get_pack_param_ids<Pack<T, I>>::vector() {
    std::vector<type_id_t> ids { };
    ids.push_back(type_info::from<T>().id());
    ids.push_back(I);
    return ids;
  }


  type_id_t get_id_from_info_getter(const type_info & (* tif)()) {
    return tif().id();
  }
}

namespace refl {
  export class field_path {
    const type_info* root_type_;
    std::vector<const field_info*> fields_;

  public:
    friend std::hash<refl::field_path>;
    friend std::formatter<refl::field_path>;

    field_path() = delete;

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
  std::size_t operator()(const refl::field_path &path) const {
    return std::hash<std::string>{}(path.to_string());
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
