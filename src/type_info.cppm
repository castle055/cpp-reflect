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

export namespace refl {
  class type_info;
  std::map<type_id_t, type_info> type_registry {};

  struct field_info {
    std::size_t index;
    std::string name;
    std::size_t size;
    std::size_t offset;
    access_spec access_type;
    [[refl::ignore]]
    std::function<const type_info&()> type;

    // Accessors
    void* get_ptr(void* obj) const {
      return static_cast<char *>(obj) + offset;
    }

    template <typename T>
    T& get_ref(void* obj) const {
      return *static_cast<T *>(get_ptr(obj));
    }
  };

  struct field_path {
    friend std::hash<refl::field_path>;

    field_path(const field_info* field): fields_{field} { }
    field_path(std::initializer_list<const field_info*> fields): fields_(fields) { }

    const type_info& type() const {
      return fields_.back()->type();
    }

    void* get_ptr(void* obj) const {
      void* ptr = obj;

      for (const field_info* field : fields_) {
        ptr = field->get_ptr(ptr);
      }

      return ptr;
    }

    template <typename T>
    T& get_ref(void* obj) const {
      return *static_cast<T *>(get_ptr(obj));
    }

    bool operator==(const field_path& other) const {
      return fields_ == other.fields_;
    }
  private:
    std::vector<const field_info*> fields_;
  };

  struct method_info {
    std::size_t index;
    std::string name;
    access_spec access_type;
  };

  template <typename T>
  struct get_pack_param_ids {
    static std::vector<type_id_t> vector() {
      return {};
    }
  };
  template <template <typename...> typename Pack, typename... Args>
  struct get_pack_param_ids<Pack<Args...>> {
    static std::vector<type_id_t> vector();
  };
  template <template <typename, std::size_t> typename Pack, typename T, std::size_t I>
  struct get_pack_param_ids<Pack<T, I>> {
    static std::vector<type_id_t> vector();
  };

  class type_info {
  private:
    template <typename Field>
    static field_info make_field_data() {
      return {
        .index = Field::index,
        .name = Field::name,
        .size = Field::size,
        .offset = Field::offset,
        .access_type = Field::access,
        .type = []() -> const type_info& { return from<typename Field::type>(); },
      };
    }

    template <typename Field>
    void push_field() {
      fields_.push_back(make_field_data<Field>());
      const auto& field = fields_.back();

      fields_by_name_[field.name] = &field;
      fields_by_offset_[field.offset] = &field;
    }

    template <typename Method>
    void push_method() {
      methods_.push_back({
        // .type        = []() -> type_info { return from<typename field<T, I>::type>(); },
        .index = Method::index,
        .name = Method::name,
        .access_type = Method::access,
      });
      const auto& method = methods_.back();

      methods_by_name_[method.name] = &method;
    }
  public:

    template <typename T>
    static const type_info& from() {
      using type = std::remove_const_t<std::remove_reference_t<T>>;
      static constexpr type_id_t tid = type_id<type>;
      static constexpr type_id_t pid = pack_type_id<type>;

      if (type_registry.contains(tid)) {
        return type_registry[tid];
      }

      if constexpr (Reflected<type>) {
        static constexpr std::size_t f_count = field_count<type>;
        static constexpr std::size_t m_count = method_count<type>;

        type_info ti{};

        [&]<std::size_t... I>(std::index_sequence<I...>) {
          (ti.push_field<field<type, I>>(), ...);
        }(std::make_index_sequence<f_count>{});

        [&]<std::size_t... I>(std::index_sequence<I...>) {
          (ti.push_method<method<type, I>>(), ...);
        }(std::make_index_sequence<m_count>{});

        ti.type_id_ = tid;
        ti.name_ = type_name<T>;

        type_registry.emplace(tid, ti);
      } else {
        type_info ti{};
        ti.type_id_ = tid;
        ti.name_ = type_name<T>;
        type_registry.emplace(tid, ti);
      }

      type_info& ti = type_registry[tid];
      if constexpr (std::is_const_v<T>) {
        ti.is_const_ = true;
      }

      if constexpr (std::is_lvalue_reference_v<T>) {
        ti.is_lval_ref_ = true;
        ti.indirect_type_id_ = from<std::remove_reference_t<T>>().id();
      } else if constexpr (std::is_rvalue_reference_v<T>) {
        ti.is_rval_ref_ = true;
        ti.indirect_type_id_ = from<std::remove_reference_t<T>>().id();
      } else if constexpr (std::is_pointer_v<T>) {
        ti.is_ptr_ = true;
        ti.indirect_type_id_ = from<std::remove_pointer_t<T>>().id();
      }

      if constexpr (pid != 0) {
        ti.pack_id_ = pid;
        ti.pack_param_ids_ = get_pack_param_ids<type>::vector();
      }

      if constexpr(std::is_copy_constructible_v<type>) {
        ti.copy_construct_function_ = [](const void* src) -> void* {
          const type& src_ref = *static_cast<const type*>(src);
          type* dest = new type(src_ref);
          return dest;
        };
      }
      if constexpr(std::is_copy_assignable_v<type>) {
        ti.copy_assign_function_ = [](void* dest, const void* src) {
          type& dest_ref = *static_cast<type*>(dest);
          const type& src_ref = *static_cast<const type*>(src);
          dest_ref = src_ref;
        };
      }
      if constexpr(Reflected<type>) {
        ti.equality_function_ = [](const void* lhs, const void* rhs) {
          const type& LHS = *static_cast<const type*>(lhs);
          const type& RHS = *static_cast<const type*>(rhs);
          return deep_eq(LHS, RHS);
        };
      } else if constexpr (std::equality_comparable<type>) {
        ti.equality_function_ = [](const void* lhs, const void* rhs) {
          const type& LHS = *static_cast<const type*>(lhs);
          const type& RHS = *static_cast<const type*>(rhs);
          return LHS == RHS;
        };
      }

      return ti;
    }

    const std::string& name() const {
      return name_;
    }

    const auto& fields() const {
      return fields_;
    }

    std::optional<const field_info*> field_by_name(const std::string& name) const {
      if (fields_by_name_.contains(name)) {
        return fields_by_name_.at(name);
      }
      return std::nullopt;
    }
    std::optional<const field_info*> field_by_offset(const std::size_t& offset) const {
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

    template <typename T>
    bool is_type() const {
      static type_id_t tid = type_id<T>;
      return type_id_ == tid;
    }

    template <template <typename...> typename Pack>
    bool is_pack() const {
      static type_id_t pid = pack_id<Pack>;
      return pack_id_ == pid;
    }

    template <template <typename T, std::size_t S> typename Pack>
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

    const type_info& indirect_type() const {
      if (indirect_type_id_.has_value()) {
        return type_registry[indirect_type_id_.value()];
      } else {
        return type_registry[0];
      }
    }

    std::vector<const type_info*> pack_parameter_types() const {
      std::vector<const type_info*> tis{};
      for (const auto& tid: pack_param_ids_) {
        tis.emplace_back(&type_registry[tid]);
      }
      return tis;
    }

    void* make_copy_of(const void* ptr) const {
      if (copy_construct_function_.has_value()) {
        return copy_construct_function_.value()(ptr);
      }
      return nullptr;
    }

    void assign_copy_of(const void* src, void* dest) const {
      if (copy_assign_function_.has_value()) {
        copy_assign_function_.value()(dest, src);
      }
    }

    bool equality(const void* lhs, const void* rhs) const {
      if (equality_function_.has_value()) {
        return equality_function_.value()(lhs, rhs);
      }
      return false;
    }

  private:
    std::string name_{};
    std::vector<field_info> fields_{};
    std::unordered_map<std::string, const field_info*> fields_by_name_{};
    std::unordered_map<std::size_t, const field_info*> fields_by_offset_{};
    std::vector<method_info> methods_{};
    std::unordered_map<std::string, const method_info*> methods_by_name_{};

    bool is_const_ = false;
    bool is_lval_ref_ = false;
    bool is_rval_ref_ = false;
    bool is_ptr_ = false;

    type_id_t type_id_{};
    std::optional<type_id_t> indirect_type_id_{std::nullopt};
    type_id_t pack_id_{};
    std::vector<type_id_t> pack_param_ids_{};

    [[refl::ignore]]
    std::optional<std::function<void*(const void*)>> copy_construct_function_{std::nullopt};
    [[refl::ignore]]
    std::optional<std::function<void(void*,const void*)>> copy_assign_function_{std::nullopt};
    [[refl::ignore]]
    std::optional<std::function<bool(const void*,const void*)>> equality_function_{std::nullopt};
  };


  template <template <typename...> typename Pack, typename... Args>
  std::vector<type_id_t> get_pack_param_ids<Pack<Args...>>::vector() {
    std::vector<type_id_t> ids{};
    (ids.push_back(type_info::from<Args>().id()), ...);
    return ids;
  }
  template <template <typename, std::size_t> typename Pack, typename T, std::size_t I>
  std::vector<type_id_t> get_pack_param_ids<Pack<T, I>>::vector() {
    std::vector<type_id_t> ids{};
    ids.push_back(type_info::from<T>().id());
    ids.push_back(I);
    return ids;
  }
}

export template<>
struct std::hash<refl::field_path> {
  std::size_t operator()(const refl::field_path& path) const {
    std::size_t seed = path.fields_.size();
    for (const auto &v: path.fields_) {
      seed ^= std::hash<std::size_t>{}(reinterpret_cast<std::size_t>(v)) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    return seed;
  }
};

