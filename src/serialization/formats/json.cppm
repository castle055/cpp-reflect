// Copyright (c) 2024-2025, Víctor Castillo Agüero.
// SPDX-License-Identifier: GPL-3.0-or-later

/*! \file  pretty_print.cppm
 *! \brief
 *!
 */

module;
#include <nlohmann/json.hpp>

export module reflect.marshal.formats.json;

import std;

import packtl;
import reflect;

import reflect.marshal.formats.base;

using JSON = nlohmann::json;

export namespace formats {
  template <typename O>
  struct json_fmt: refl::visitor<json_fmt<O>> {
    struct args_t {
      bool         pretty = false;
      unsigned int indent = 2;
    };

    explicit json_fmt(O& out_, args_t args_)
        : refl::visitor<json_fmt<O>>(),
          out(out_),
          args(args_) {}

    template <typename T>
    void handle_pointer(const T* it) {
      if constexpr (std::same_as<T, char>) {
        this->handle_value(it);
        return;
      }

      current() = "reference";
      // this->visit_pointer(it);
    }

    template <typename T>
    void handle_reference(const T& it) {
      current() = "reference";
      // this->visit_pointer(it);
    }

    template <typename T>
    void handle_value(const T& it) {
      if constexpr (std::is_same_v<T, std::atomic_flag>) {
        current() = (it.test() ? "SET" : "CLEAR");
      } else if constexpr (packtl::is_type<std::unique_ptr, T>::value) {
        const auto* value = it.get();
        this->handle_value(*value);
        return;
      } else if constexpr (packtl::is_type<std::shared_ptr, T>::value) {
        if (current_policy == serialize::policy::deep) {
          const auto* value = it.get();
          this->handle_value(*value);
          return;
        } else {
          current() = "reference";
          return;
        }
      } else if constexpr (packtl::is_type<std::weak_ptr, T>::value) {
        if (it.expired()) {
          current() = "null";
          return;
        } else if (current_policy == serialize::policy::deep) {
          const auto* value = it.get();
          this->handle_value(*value);
          return;
        } else {
          current() = "reference";
          return;
        }
      } else if constexpr (refl::Reflected<T>) {
        if (visited_.contains((std::size_t)&it)) {
          out << "<circular reference>";
          return;
        }
        visited_.emplace((std::size_t)&it);
      } else if constexpr (std::is_convertible_v<T, std::string>) {
        current() = std::format("{}", std::string{it});
      } else if constexpr (std::same_as<T, char*>) {
        current() = std::format("{}", std::string{it});
        return;
      } else if constexpr (std::same_as<T, const char*>) {
        current() = std::format("{}", std::string{it});
        return;
      } else if constexpr (std::same_as<T, int> or std::same_as<T, unsigned int> or
                           std::same_as<T, short> or std::same_as<T, unsigned short> or
                           std::same_as<T, long> or std::same_as<T, unsigned long> or
                           std::same_as<T, float> or std::same_as<T, double>) {
        current() = it;
      } else if constexpr (std::same_as<T, bool>) {
        current() = std::format("{}", it ? "true" : "false");
      } else if constexpr (std::formattable<T, char>) {
        current() = std::format("{}", it);
      }

      this->visit_value(it);
    }

    template <typename T>
    void handle_iterable(const T& iterable) {
      if constexpr (std::__is_std_pair<typename T::value_type>) {
        using type = typename T::value_type;
        if constexpr (std::same_as<typename type::first_type, std::string> or
                      std::same_as<typename type::first_type, const std::string>) {
          current() = JSON::object({});
          for (const auto& [first, second]: iterable) {
            current()[first] = JSON{};
            push(current()[first]);
            this->handle_value(second);
            pop();
          }
          return;
        }
      }
      current() = JSON::array({});
      push(current());
      this->visit_iterable(iterable);
      pop();
    }

    template <typename T>
    void handle_iterable_element(const T& element) {
      current().push_back(JSON{});
      push(current().back());
      this->visit_iterable_element(element);
      pop();
    }

    template <typename T>
    void handle_tuple(const T& tuple) {
      if constexpr (std::__is_std_pair<T> and std::same_as<typename T::first_type, std::string>) {
        current()              = JSON::object({});
        current()[tuple.first] = JSON{};
        push(current()[tuple.first]);
        this->handle_value(tuple.second);
        pop();
      } else {
        current() = JSON::array({});
        push(current());
        this->visit_tuple(tuple);
        pop();
      }
    }

    template <typename T>
    void handle_tuple_element(const T& element) {
      current().push_back(JSON{});
      push(current().back());
      this->visit_tuple_element(element);
      pop();
    }

    template <typename T, typename Field>
    void handle_field(const T& obj) {
      std::string field_name = Field::name;
      if constexpr (Field::template has_metadata<serialize::name>) {
        field_name = Field::template get_metadata<serialize::name>.value;
      }

      current_policy = serialize::policy::shallow;
      if constexpr (Field::template has_metadata<serialize::policy::policy_e>) {
        current_policy = Field::template get_metadata<serialize::policy::policy_e>;
        if constexpr (Field::template get_metadata<serialize::policy::policy_e> ==
                      serialize::policy::deep) {
          current()[field_name] = JSON{};
          push(current()[field_name]);

          if constexpr (Field::is_reference) {
            const auto& it = Field::from_instance(obj);
            this->handle_value(it);
          } else if constexpr (Field::is_pointer) {
            const auto* it = Field::from_instance(obj);
            this->handle_value(*it);
          } else {
            const auto& it = Field::from_instance(obj);
            this->handle_value(it);
          }

          pop();
        } else if constexpr (Field::template get_metadata<serialize::policy::policy_e> ==
                             serialize::policy::shallow) {
          current()[field_name] = JSON{};
          push(current()[field_name]);

          if constexpr (Field::is_reference) {
            current() = "reference";
          } else if constexpr (Field::is_pointer) {
            current() = "reference";
          } else {
            const auto& it = Field::from_instance(obj);
            this->handle_value(it);
          }

          pop();
        } else if constexpr (Field::template get_metadata<serialize::policy::policy_e> ==
                             serialize::policy::skip) {
          // do nothing
        }
      } else {
        current()[field_name] = JSON{};
        push(current()[field_name]);

        this->template visit_obj_field<T, Field>(obj);

        pop();
      }
    }

    template <typename T>
    void handle_obj(const T& obj) {
      // current()["__type"] = refl::type_name<T>;
      this->visit_obj(obj);
    }

    template <refl::Reflected R>
    void serialize(const R& obj) {
      json_ = JSON::object({});
      push(json_);
      this->visit(obj);
      out << json_.dump(args.pretty ? args.indent : -1);
    }

  private:
    JSON& current() {
      return *obj_stack.top();
    }

    void push(JSON& obj) {
      obj_stack.push(&obj);
    }

    void pop() {
      obj_stack.pop();
    }

  private:
    std::unordered_set<std::size_t> visited_{};
    O&                              out;
    args_t                          args;
    JSON                            json_;
    std::stack<JSON*>               obj_stack;

    serialize::policy::policy_e current_policy{serialize::policy::shallow};
  };
} // namespace formats
