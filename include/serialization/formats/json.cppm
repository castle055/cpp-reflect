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
export import reflect;
import packtl;

import reflect.marshal.formats.base;

export namespace refl {
  using JSON = nlohmann::json;
}

export namespace formats {
  template<typename O>
  struct json_fmt: refl::visitor<json_fmt<O>> {
    struct args_t {
      bool pretty         = false;
      unsigned int indent = 2;
    };

    explicit json_fmt(O &out_, args_t args_)
      : refl::visitor<json_fmt<O>>(),
        out(out_),
        args(args_) {
    }

    template<typename T>
    void handle_pointer(const T* it) {
      if constexpr (std::same_as<T, char>) {
        this->handle_value(it);
        return;
      }

      current() = "reference";
      // this->visit_pointer(it);
    }

    template<typename T>
    void handle_reference(const T &it) {
      current() = "reference";
      // this->visit_pointer(it);
    }

    template<typename T>
    void handle_value(const T &it) {
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
        current() = std::format("{}", std::string {it});
      } else if constexpr (std::same_as<T, char*>) {
        current() = std::format("{}", std::string {it});
        return;
      } else if constexpr (std::same_as<T, const char*>) {
        current() = std::format("{}", std::string {it});
        return;
      } else if constexpr (std::same_as<T, int> or std::same_as<T, unsigned int> or
                           std::same_as<T, short> or std::same_as<T, unsigned short> or
                           std::same_as<T, long> or std::same_as<T, unsigned long> or
                           std::same_as<T, float> or std::same_as<T, double>) {
        current() = it;
      } else if constexpr (std::same_as<T, bool>) {
        current() = it ? true : false;
      } else if constexpr (std::formattable<T, char>) {
        current() = std::format("{}", it);
      }

      this->visit_value(it);
    }

    template<typename T>
    void handle_iterable(const T &iterable) {
      if constexpr (std::__is_pair<typename T::value_type>) {
        using type = typename T::value_type;
        if constexpr (std::same_as<typename type::first_type, std::string> or
                      std::same_as<typename type::first_type, const std::string>) {
          current() = refl::JSON::object({ });
          for (const auto &[first, second]: iterable) {
            current()[first] = refl::JSON { };
            push(current()[first]);
            this->handle_value(second);
            pop();
          }
          return;
        }
      }
      current() = refl::JSON::array({ });
      push(current());
      this->visit_iterable(iterable);
      pop();
    }

    template<typename T>
    void handle_iterable_element(const T &element) {
      current().push_back(refl::JSON { });
      push(current().back());
      this->visit_iterable_element(element);
      pop();
    }

    template<typename T>
    void handle_tuple(const T &tuple) {
      if constexpr (std::__is_pair<T> and std::same_as<typename T::first_type, std::string>) {
        current()              = refl::JSON::object({ });
        current()[tuple.first] = refl::JSON { };
        push(current()[tuple.first]);
        this->handle_value(tuple.second);
        pop();
      } else {
        current() = refl::JSON::array({ });
        push(current());
        this->visit_tuple(tuple);
        pop();
      }
    }

    template<typename T>
    void handle_tuple_element(const T &element) {
      current().push_back(refl::JSON { });
      push(current().back());
      this->visit_tuple_element(element);
      pop();
    }

    template<typename T, typename Field>
    void handle_field(const T &obj) {
      std::string field_name = Field::name;
      if constexpr (Field::template has_metadata<serialize::name>) {
        field_name = Field::template get_metadata<serialize::name>.value;
      }

      current_policy = serialize::policy::shallow;
      if constexpr (Field::template has_metadata<serialize::policy::policy_e>) {
        current_policy = Field::template get_metadata<serialize::policy::policy_e>;
        if constexpr (Field::template get_metadata<serialize::policy::policy_e> ==
                      serialize::policy::deep) {
          current()[field_name] = refl::JSON { };
          push(current()[field_name]);

          if constexpr (Field::is_reference) {
            const auto &it = Field::from_instance(obj);
            this->handle_value(it);
          } else if constexpr (Field::is_pointer) {
            const auto* it = Field::from_instance(obj);
            this->handle_value(*it);
          } else {
            const auto &it = Field::from_instance(obj);
            this->handle_value(it);
          }

          pop();
        } else if constexpr (Field::template get_metadata<serialize::policy::policy_e> ==
                             serialize::policy::shallow) {
          current()[field_name] = refl::JSON { };
          push(current()[field_name]);

          if constexpr (Field::is_reference) {
            current() = "reference";
          } else if constexpr (Field::is_pointer) {
            current() = "reference";
          } else {
            const auto &it = Field::from_instance(obj);
            this->handle_value(it);
          }

          pop();
        } else if constexpr (Field::template get_metadata<serialize::policy::policy_e> ==
                             serialize::policy::skip) {
          // do nothing
        }
      } else {
        current()[field_name] = refl::JSON { };
        push(current()[field_name]);

        this->template visit_obj_field<T, Field>(obj);

        pop();
      }
    }

    template<typename T>
    void handle_obj(const T &obj) {
      // current()["__type"] = refl::type_name<T>;
      this->visit_obj(obj);
    }

    template<refl::Reflected R>
    void serialize(const R &obj) {
      json_ = refl::JSON::object({ });
      push(json_);
      this->visit(obj);
      out << json_.dump(args.pretty ? args.indent : -1);
    }

    static refl::any deserialize(const std::string_view &str) {
      refl::JSON obj = refl::JSON::parse(str);

      if (obj.is_object()) {
        refl::archive arc { };
        deserialize_object(arc, obj);
        return arc;
      }
      if (obj.is_array()) {
        std::vector<refl::any> vec { };
        deserialize_array(vec, obj);
        return vec;
      }

      throw std::invalid_argument(std::format("Invalid JSON object: {}", obj.dump()));
    }

    static refl::any deserialize(const refl::JSON &obj) {
      if (obj.is_object()) {
        refl::archive arc { };
        deserialize_object(arc, obj);
        return arc;
      }
      if (obj.is_array()) {
        std::vector<refl::any> vec { };
        deserialize_array(vec, obj);
        return vec;
      }

      throw std::invalid_argument(std::format("Invalid JSON object: {} {}", obj.type_name(), obj.dump()));
    }

  private:
    static void deserialize_object(refl::archive &arc, const refl::JSON &obj) {
      for (const auto &it: obj.items()) {
        const std::string &key  = it.key();
        const refl::JSON &value = it.value();
        if (value.is_object()) {
          arc[key] = refl::archive { };
          deserialize_object(arc[key].as<refl::archive>(), value);
        } else if (value.is_array()) {
          arc[key] = std::vector<refl::any> { };
          deserialize_array(arc[key].as<std::vector<refl::any>>(), value);
        } else if (value.is_boolean()) {
          arc[key] = bool {value};
        } else if (value.is_number_integer()) {
          arc[key] = std::int64_t {value};
        } else if (value.is_number_unsigned()) {
          arc[key] = std::uint64_t {value};
        } else if (value.is_number_float()) {
          arc[key] = std::double_t {value};
        } else if (value.is_string()) {
          arc[key] = std::string {value};
        } else if (value.is_null()) {
          arc[key] = std::string {"null"};
        } else {
          throw std::invalid_argument(std::format("Invalid JSON object: {}", value.dump()));
        }
      }
    }

    static void deserialize_array(auto &vec, const refl::JSON &obj) {
      for (auto &it: obj) {
        if (it.is_object()) {
          vec.emplace_back(refl::archive { });
          deserialize_object(vec.back().template as<refl::archive>(), it);
        } else if (it.is_array()) {
          vec.emplace_back(std::vector<refl::any> { });
          deserialize_array(vec.back().template as<std::vector<refl::any>>(), it);
        } else if (it.is_boolean()) {
          vec.emplace_back(bool {it});
        } else if (it.is_number_integer()) {
          vec.emplace_back(std::int64_t {it});
        } else if (it.is_number_unsigned()) {
          vec.emplace_back(std::uint64_t {it});
        } else if (it.is_number_float()) {
          vec.emplace_back(std::double_t {it});
        } else if (it.is_string()) {
          vec.emplace_back(std::string {it});
        } else if (it.is_null()) {
          vec.emplace_back(std::string {"null"});
        } else {
          throw std::invalid_argument(std::format("Invalid JSON object: {}", it.dump()));
        }
      }
    }

    refl::JSON &current() {
      return *obj_stack.top();
    }

    void push(refl::JSON &obj) {
      obj_stack.push(&obj);
    }

    void pop() {
      obj_stack.pop();
    }

  private:
    std::unordered_set<std::size_t> visited_ { };
    O &out;
    args_t args;
    refl::JSON json_;
    std::stack<refl::JSON*> obj_stack;

    serialize::policy::policy_e current_policy {serialize::policy::shallow};
  };
} // namespace formats
