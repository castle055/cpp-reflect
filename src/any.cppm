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
      requires (not std::same_as<T, any>)
    explicit any(T* t) {
      data_       = t;
      destructor_ = [](void* ptr) { delete static_cast<T*>(ptr); };
      type_info_  = &type_info::from<T>();
    }

  public:
    any() {
      data_       = nullptr;
      destructor_ = [](void*) {};
      type_info_  = nullptr;
    }

    template<typename T>
      requires (not std::same_as<T, any>)
    any(const T &t) {
      data_       = new T(t);
      destructor_ = [](void* ptr) { delete static_cast<T*>(ptr); };
      type_info_  = &type_info::from<T>();
    }

    template<typename T>
      requires (not std::same_as<T, any>)
    any(T &&t) {
      data_       = new T(std::forward<T>(t));
      destructor_ = [](void* ptr) { delete static_cast<T*>(ptr); };
      type_info_  = &type_info::from<T>();
    }

    template<typename T, typename... Args>
    static any make(Args &&args) {
      auto* ptr = new T(std::forward<Args>(args)...);
      return any {ptr};
    }

    ~any() {
      destructor_(data_);
      data_ = nullptr;
    }

    any(const any &other) {
      type_info_  = other.type_info_;
      data_       = type_info_->make_copy_of(other.data_);
      destructor_ = other.destructor_;
    }

    any &operator=(const any &other) {
      type_info_ = other.type_info_;
      type_info_->assign_copy_of(other.data_, data_);
      destructor_ = other.destructor_;
      return *this;
    }

    template<typename T>
    bool is() const {
      if (type_info_ == nullptr) return false;
      return type_info_->is_type<T>();
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

  private:
    void* data_;
    std::function<void(void*)> destructor_;
    const type_info* type_info_;
  };
}
