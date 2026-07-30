/*! \file  TypeInfo.cppm
 *! \brief
 *!
 */
module;
#include <meta>

export module refl.descriptors:TypeInfo;

import std;

import :FundamentalInfo;
import :FieldInfo;
import :ClassInfo;
import :PointerInfo;
import :EnumInfo;
import :FunctionInfo;


namespace refl {
  export using TypeError = std::runtime_error;

  export using TypeDescriptor =    //
          std::variant<            //
                  FundamentalInfo, //
                  ClassInfo,       //
                  EnumInfo,        //
                  FunctionInfo,    //
                  PointerInfo,     //
                  ReferenceInfo    //
                  >;

  export class TypeOperations {
    std::string_view name{};

    void (*fn_default_constructor_)(void *){nullptr};

    void (*fn_destructor_)(void *){nullptr};

    void (*fn_copy_construct_)(void *, const void *){nullptr};

    void (*fn_copy_assign_)(void *, const void *){nullptr};

    void (*fn_move_construct_)(void *, void *){nullptr};

    void (*fn_move_assign_)(void *, void *){nullptr};

    bool (*fn_equality_)(const void *, const void *){nullptr};

  public:
    template<typename T>
    void init(std::string_view type_name) {
      name = type_name;

      // Special member functions
      if constexpr (std::is_default_constructible_v<T>) {
        fn_default_constructor_ = [](void *dst) { std::construct_at<T>(static_cast<T *>(dst)); };
      }
      if constexpr (std::is_destructible_v<T>) {
        fn_destructor_ = [](void *ptr) { std::destroy_at<T>(static_cast<T *>(ptr)); };
      }
      if constexpr (std::is_copy_constructible_v<T>) {
        fn_copy_construct_ = [](void *dst, const void *src) {
          std::construct_at<T>(static_cast<T *>(dst), *static_cast<const T *>(src));
        };
      }
      if constexpr (std::is_copy_assignable_v<T>) {
        fn_copy_assign_ = [](void *dst, const void *src) { *static_cast<T *>(dst) = *static_cast<const T *>(src); };
      }
      if constexpr (std::is_move_constructible_v<T>) {
        fn_move_construct_ = [](void *dst, void *src) {
          std::construct_at<T>(static_cast<T *>(dst), std::move(*static_cast<T *>(src)));
        };
      }
      if constexpr (std::is_move_assignable_v<T>) {
        fn_move_assign_ = [](void *dst, void *src) { *static_cast<T *>(dst) = std::move(*static_cast<T *>(src)); };
      }
      if constexpr (std::equality_comparable<T>) {
        fn_equality_ = [](const void *lhs, const void *rhs) -> bool {
          return *static_cast<const T *>(lhs) == *static_cast<const T *>(rhs);
        };
      }
    }

  public:
    friend TypeInfo;

    bool is_default_constructible() const { return fn_default_constructor_ != nullptr; }

    bool is_destructible() const { return fn_destructor_ != nullptr; }

    bool is_copy_constructible() const { return fn_copy_construct_ != nullptr; }

    bool is_copy_assignable() const { return fn_copy_assign_ != nullptr; }

    bool is_move_constructible() const { return fn_move_construct_ != nullptr; }

    bool is_move_assignable() const { return fn_move_assign_ != nullptr; }

    bool is_eq_comparable() const { return fn_equality_ != nullptr; }

    void construct_at(void *dst) const {
      assert_valid_fn(fn_default_constructor_, "default constructor");
      return fn_default_constructor_(dst);
    }

    void destructor(void *ptr) const {
      assert_valid_fn(fn_destructor_, "destructor");
      return fn_destructor_(ptr);
    }

    void copy_construct(void *dst, const void *src) const {
      assert_valid_fn(fn_copy_construct_, "copy constructor");
      return fn_copy_construct_(dst, src);
    }

    void copy_assign(void *dst, const void *src) const {
      assert_valid_fn(fn_copy_assign_, "copy assignment");
      fn_copy_assign_(dst, src);
    }

    void move_construct(void *dst, void *src) const {
      assert_valid_fn(fn_move_construct_, "move constructor");
      return fn_move_construct_(dst, src);
    }

    void move_assign(void *dst, void *src) const {
      assert_valid_fn(fn_move_assign_, "move assignment");
      fn_move_assign_(dst, src);
    }

    bool equality(const void *lhs, const void *rhs) const {
      assert_valid_fn(fn_equality_, "equality");
      return fn_equality_(lhs, rhs);
    }

  private:
    void assert_valid_fn(auto *fn_ptr, std::string_view what) const {
      if (nullptr == fn_ptr)
        throw_type_error(std::format("has no {}", what));
    }

    void throw_type_error(std::string_view what) const { throw TypeError{std::format("Type '{}': {}", name, what)}; }
  };

  export struct TypeInfo {
    enum Kind {
      FUNDAMENTAL,
      CLASS,
      POINTER,
      REFERENCE,
      ENUM,
      FUNCTION,
    } kind{};

    std::string_view name{};
    std::size_t      size{};
    std::size_t      alignment{};
    TypeOperations   operations{};
    TypeDescriptor   descriptor{};

    TypeInfo() = default;

    TypeInfo(const TypeInfo &) = delete;

    TypeInfo &operator=(const TypeInfo &) = delete;

    TypeInfo(TypeInfo &&) = default;

    TypeInfo &operator=(TypeInfo &&) = delete; // Some descriptors cannot be moved

    template<Kind K>
    decltype(auto) as() const {
      if (K != kind) {
        throw std::runtime_error{std::format("Cannot describe {} as {}", enum_to_string(kind), enum_to_string(K))};
      }
      if constexpr (K == FUNDAMENTAL) {
        return std::get<FundamentalInfo>(descriptor);
      } else if constexpr (K == CLASS) {
        return std::get<ClassInfo>(descriptor);
      } else if constexpr (K == ENUM) {
        return std::get<EnumInfo>(descriptor);
      } else if constexpr (K == POINTER) {
        return std::get<PointerInfo>(descriptor);
      } else if constexpr (K == REFERENCE) {
        return std::get<ReferenceInfo>(descriptor);
      } else if constexpr (K == FUNCTION) {
        return std::get<FunctionInfo>(descriptor);
      }
    }

  private:
    void throw_type_error(std::string_view what) const { throw TypeError{std::format("Type '{}': {}", name, what)}; }
  };
} // namespace refl
