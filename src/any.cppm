/*! \file  any.cppm
 *! \brief 
 *!
 */

export module reflect:any;

import std;

export import :type_info;

namespace refl {
  export class any {
  private:
    template<typename T>
      requires (not std::same_as<std::remove_reference_t<T>, any>)
    explicit any(T* t) {
      using type = std::remove_const_t<std::remove_reference_t<T>>;
      data_       = t;
      destructor_ = [](void* ptr) { delete static_cast<type*>(ptr); };
      type_info_  = &type_info::from<type>();
    }

  public:
    any() {
      data_       = nullptr;
      destructor_ = [](void*) {};
      type_info_  = nullptr;
    }

    template<typename T>
      requires (not std::same_as<std::remove_reference_t<T>, any>)
    any(const T &t) {
      using type = std::remove_const_t<std::remove_reference_t<T>>;
      data_       = new type(t);
      destructor_ = [](void* ptr) { delete static_cast<T*>(ptr); };
      type_info_  = &type_info::from<type>();
    }

    template<typename T>
      requires (not std::same_as<std::remove_reference_t<T>, any>)
    any(T &&t) {
      using type = std::remove_const_t<std::remove_reference_t<T>>;
      data_       = new type(std::forward<T>(t));
      destructor_ = [](void* ptr) { delete static_cast<T*>(ptr); };
      type_info_  = &type_info::from<type>();
    }

    template<typename T, typename... Args>
      requires (not std::is_reference_v<T>)
    static any make(Args&& ...args) {
      using type = std::remove_const_t<std::remove_reference_t<T>>;
      auto* ptr = new type(std::forward<Args>(args)...);
      return any {ptr};
    }

    template<typename T>
      requires (not std::same_as<std::remove_reference_t<T>, any>)
    static any make(const T& value) {
      using type = std::remove_const_t<std::remove_reference_t<T>>;
      auto* ptr = new type(value);
      return any {ptr};
    }

    static any make(const refl::type_info& t_info, void* ptr) {
      any a{};
      a.data_ = t_info.make_copy_of(ptr);
      a.type_info_ = &t_info;
      return a;
    }

    ~any() {
      destructor_(data_);
      data_ = nullptr;
    }

    any(const any &other) {
      type_info_  = other.type_info_;
      if (type_info_ != nullptr) {
        data_ = type_info_->make_copy_of(other.data_);
      }
      destructor_ = other.destructor_;
    }

    any &operator=(const any &other) {
      if (data_ != nullptr) {
        destructor_(data_);
        data_ = nullptr;
      }
      type_info_ = other.type_info_;
      if (type_info_ != nullptr) {
        data_ = type_info_->make_copy_of(other.data_);
      }
      destructor_ = other.destructor_;
      return *this;
    }

    template<typename T>
    bool is() const {
      if (type_info_ == nullptr) return false;
      return type_info_->is_type<T>();
    }

    bool is(const refl::type_info& tinfo) const {
      if (type_info_ == nullptr) return false;
      return type_info_->id() == tinfo.id();
    }

    template<typename T>
    T &as() {
      if (not is<T>()) { throw std::bad_cast(); }
      return *static_cast<T*>(data_);
    }

    template<typename T>
    const T &as() const {
      if (not is<T>()) { throw std::bad_cast(); }
      return *static_cast<T*>(data_);
    }

    const type_info &type() const {
      if (type_info_ == nullptr) { throw std::bad_typeid(); }
      return *type_info_;
    }

    bool is_null() const {
      return data_ == nullptr or type_info_ == nullptr;
    }

    void* data() {
      return data_;
    }

    const void* data() const {
      return data_;
    }

    bool operator==(const any &other) const {
      if (data_ == nullptr) {
        return type_info_ == other.type_info_;
      }
      return type_info_ == other.type_info_ and type_info_->equality(data_, other.data_);
    }

  private:
    void* data_;
    std::function<void(void*)> destructor_;
    const type_info* type_info_;
  };

  export class any_ref {
  public:
    any_ref() {
      data_       = nullptr;
      type_info_  = nullptr;
    }

    template<typename T>
      requires (not std::same_as<std::remove_reference_t<T>, any_ref> and not std::same_as<std::remove_reference_t<T>, any>)
    any_ref(T &t) {
      using type = std::remove_const_t<std::remove_reference_t<T>>;
      data_       = &t;
      type_info_  = &type_info::from<type>();
    }

    template<typename T>
      requires (not std::same_as<std::remove_reference_t<T>, any_ref> and not std::same_as<std::remove_reference_t<T>, any>)
    any_ref(T *t) {
      using type = std::remove_const_t<std::remove_reference_t<T>>;
      data_       = t;
      type_info_  = &type_info::from<type>();
    }

    any_ref(const refl::type_info& t_info, void* ptr) {
      any_ref a{};
      a.data_ = ptr;
      a.type_info_ = &t_info;
    }

    any_ref(refl::any& owner_any) {
      any_ref a{};
      a.data_ = owner_any.data();
      a.type_info_ = &owner_any.type();
    }


    ~any_ref() {
      data_ = nullptr;
    }

    any_ref(const any_ref &other) {
      type_info_  = other.type_info_;
      if (type_info_ != nullptr) {
        data_ = type_info_->make_copy_of(other.data_);
      }
    }

    any_ref &operator=(const any_ref &other) {
      if (data_ != nullptr) {
        data_ = nullptr;
      }
      type_info_ = other.type_info_;
      if (type_info_ != nullptr) {
        data_ = type_info_->make_copy_of(other.data_);
      }
      return *this;
    }

    template<typename T>
    bool is() const {
      if (type_info_ == nullptr) return false;
      return type_info_->is_type<T>();
    }

    bool is(const refl::type_info& tinfo) const {
      if (type_info_ == nullptr) return false;
      return type_info_->id() == tinfo.id();
    }

    template<typename T>
    T &as() {
      if (not is<T>()) { throw std::bad_cast(); }
      return *static_cast<T*>(data_);
    }

    template<typename T>
    const T &as() const {
      if (not is<T>()) { throw std::bad_cast(); }
      return *static_cast<T*>(data_);
    }

    const type_info &type() const {
      if (type_info_ == nullptr) { throw std::bad_typeid(); }
      return *type_info_;
    }

    bool is_null() const {
      return data_ == nullptr or type_info_ == nullptr;
    }

    void* data() {
      return data_;
    }

    const void* data() const {
      return data_;
    }

  private:
    void* data_;
    const type_info* type_info_;
  };
}
